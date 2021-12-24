// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php.

#include "primitives/transaction.h"

#include "hash.h"
#include "script/standard.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

std::string BaseOutPoint::ToStringShort() const
{
    return strprintf("%s-%u", hash.ToString().substr(0,64), n);
}

std::string COutPoint::ToString() const
{
    return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10), n);
}

std::string SaplingOutPoint::ToString() const
{
    return strprintf("SaplingOutPoint(%s, %u)", hash.ToString().substr(0, 10), n);
}


CTxIn::CTxIn(COutPoint prevoutIn, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

CTxIn::CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string CTxIn::ToString() const
{
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    else
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    if (nSequence != std::numeric_limits<unsigned int>::max())
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

CTxOut::CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
    nRounds = -10;
}

uint256 CTxOut::GetHash() const
{
    return SerializeHash(*this);
}

bool CTxOut::GetKeyIDFromUTXO(CKeyID& keyIDRet) const
{
    std::vector<valtype> vSolutions;
    txnouttype whichType;
    if (scriptPubKey.empty() || !Solver(scriptPubKey, whichType, vSolutions))
        return false;
    if (whichType == TX_PUBKEY) {
        keyIDRet = CPubKey(vSolutions[0]).GetID();
        return true;
    }
    if (whichType == TX_PUBKEYHASH || whichType == TX_COLDSTAKE) {
        keyIDRet = CKeyID(uint160(vSolutions[0]));
        return true;
    }
    return false;
}


std::string CTxOut::ToString() const
{
    return strprintf("CTxOut(nValue=%d.%06d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0,30));
}

CMutableTransaction::CMutableTransaction() : nVersion(CTransaction::CURRENT_VERSION), nType(CTransaction::TxType::NORMAL), nLockTime(0) {}
CMutableTransaction::CMutableTransaction(const CTransaction& tx) : nVersion(tx.nVersion), nType(tx.nType), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), sapData(tx.sapData), extraPayload(tx.extraPayload) {}

uint256 CMutableTransaction::GetHash() const
{
    return SerializeHash(*this);
}

uint256 CTransaction::ComputeHash() const
{
    return SerializeHash(*this);
}

size_t CTransaction::DynamicMemoryUsage() const
{
    return memusage::RecursiveDynamicUsage(vin) + memusage::RecursiveDynamicUsage(vout);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
CTransaction::CTransaction() : nVersion(CTransaction::CURRENT_VERSION), nType(TxType::NORMAL), vin(), vout(), nLockTime(0), hash() {}
CTransaction::CTransaction(const CMutableTransaction &tx) : nVersion(tx.nVersion), nType(tx.nType), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), sapData(tx.sapData), extraPayload(tx.extraPayload), hash(ComputeHash()) {}
CTransaction::CTransaction(CMutableTransaction &&tx) : nVersion(tx.nVersion), nType(tx.nType), vin(std::move(tx.vin)), vout(std::move(tx.vout)), nLockTime(tx.nLockTime), sapData(tx.sapData), extraPayload(tx.extraPayload), hash(ComputeHash()) {}


bool CTransaction::IsCoinStake() const
{
    if (vin.empty())
        return false;

    if (vin[0].prevout.IsNull())
        return false;

    return (vout.size() >= 2 && vout[0].IsEmpty());
}

bool CTransaction::HasP2CSOutputs() const
{
    for(const CTxOut& txout : vout) {
        if (txout.scriptPubKey.IsPayToColdStaking())
            return true;
    }
    return false;
}

CAmount CTransaction::GetValueOut() const
{
    CAmount nValueOut = 0;
    for (std::vector<CTxOut>::const_iterator it(vout.begin()); it != vout.end(); ++it) {
        // ISLAMIC_DIGITAL_COIN: previously MoneyRange() was called here. This has been replaced with negative check and boundary wrap check.
        if (it->nValue < 0)
            throw std::runtime_error("CTransaction::GetValueOut() : value out of range : less than 0");

        if ((nValueOut + it->nValue) < nValueOut)
            throw std::runtime_error("CTransaction::GetValueOut() : value out of range : wraps the int64_t boundary");

        nValueOut += it->nValue;
    }

    // Sapling
    if (hasSaplingData() && sapData->valueBalance < 0) {
        // NB: negative valueBalance "takes" money from the transparent value pool just as outputs do
        nValueOut += -sapData->valueBalance;

        // Verify Sapling version
        if (!isSaplingVersion())
            throw std::runtime_error("GetValueOut(): invalid tx version");
    }

    return nValueOut;
}

CAmount CTransaction::GetShieldedValueIn() const
{
    CAmount nValue = 0;

    if (hasSaplingData() && sapData->valueBalance > 0) {
        // NB: positive valueBalance "gives" money to the transparent value pool just as inputs do
        nValue += sapData->valueBalance;

        // Verify Sapling
        if (!isSaplingVersion())
            throw std::runtime_error("GetValueOut(): invalid tx version");
    }

    return nValue;
}


double CTransaction::ComputePriority(double dPriorityInputs, unsigned int nTxSize) const
{
    nTxSize = CalculateModifiedSize(nTxSize);
    if (nTxSize == 0) return 0.0;

    return dPriorityInputs / nTxSize;
}

unsigned int CTransaction::CalculateModifiedSize(unsigned int nTxSize) const
{
    // In order to avoid disincentivizing cleaning up the UTXO set we don't count
    // the constant overhead for each txin and up to 110 bytes of scriptSig (which
    // is enough to cover a compressed pubkey p2sh redemption) for priority.
    // Providing any more cleanup incentive than making additional inputs free would
    // risk encouraging people to create junk outputs to redeem later.
    if (nTxSize == 0)
        nTxSize = ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
    for (std::vector<CTxIn>::const_iterator it(vin.begin()); it != vin.end(); ++it)
    {
        unsigned int offset = 41U + std::min(110U, (unsigned int)it->scriptSig.size());
        if (nTxSize > offset)
            nTxSize -= offset;
    }
    return nTxSize;
}

unsigned int CTransaction::GetTotalSize() const
{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

std::string CTransaction::ToString() const
{
    std::ostringstream ss;
    ss << "CTransaction(hash=" << GetHash().ToString().substr(0, 10)
       << ", ver=" << nVersion
       << ", type=" << nType
       << ", vin.size=" << vin.size()
       << ", vout.size=" << vout.size()
       << ", nLockTime=" << nLockTime;
    if (IsShieldedTx()) {
        ss << ", valueBalance=" << sapData->valueBalance
           << ", vShieldedSpend.size=" << sapData->vShieldedSpend.size()
           << ", vShieldedOutput.size=" << sapData->vShieldedOutput.size();
    }
    if (IsSpecialTx()) {
        ss << ", extraPayload.size=" << extraPayload->size();
    }
    ss << ")\n";
    for (unsigned int i = 0; i < vin.size(); i++)
        ss << "    " << vin[i].ToString() << "\n";
    for (unsigned int i = 0; i < vout.size(); i++)
        ss << "    " << vout[i].ToString() << "\n";
    return ss.str();
}
