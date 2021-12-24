// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MASTERNODE_H
#define MASTERNODE_H

#include "base58.h"
#include "key.h"
#include "messagesigner.h"
#include "net.h"
#include "serialize.h"
#include "sync.h"
#include "timedata.h"
#include "util.h"

/* Depth of the block pinged by masternodes */
static const unsigned int MNPING_DEPTH = 12;

/* Masternode collateral amount */
static const CAmount MN_COLL_AMT = 10000 * COIN;


class CMasternode;
class CMasternodeBroadcast;
class CMasternodePing;

int MasternodeMinPingSeconds();
int MasternodeBroadcastSeconds();
int MasternodeCollateralMinConf();
int MasternodePingSeconds();
int MasternodeExpirationSeconds();
int MasternodeRemovalSeconds();

//
// The Masternode Ping Class : Contains a different serialize method for sending pings from masternodes throughout the network
//

class CMasternodePing : public CSignedMessage
{
public:
    CTxIn vin;
    uint256 blockHash;
    int64_t sigTime; //mnb message times

    CMasternodePing();
    CMasternodePing(const CTxIn& newVin, const uint256& nBlockHash, uint64_t _sigTime);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(vin);
        READWRITE(blockHash);
        READWRITE(sigTime);
        READWRITE(vchSig);
        try {
            READWRITE(nMessVersion);
        } catch (...) {
            nMessVersion = MessageVersion::MESS_VER_STRMESS;
        }
    }

    uint256 GetHash() const;

    // override CSignedMessage functions
    uint256 GetSignatureHash() const override { return GetHash(); }
    std::string GetStrMessage() const override;
    const CTxIn GetVin() const { return vin; };
    bool IsNull() const { return blockHash.IsNull() || vin.prevout.IsNull(); }

    bool CheckAndUpdate(int& nDos, bool fRequireAvailable = true, bool fCheckSigTimeOnly = false);
    void Relay();

    CMasternodePing& operator=(const CMasternodePing& other) = default;

    friend bool operator==(const CMasternodePing& a, const CMasternodePing& b)
    {
        return a.vin == b.vin && a.blockHash == b.blockHash;
    }
    friend bool operator!=(const CMasternodePing& a, const CMasternodePing& b)
    {
        return !(a == b);
    }
};

//
// The Masternode Class. It contains the input of the 10000 IDC, signature to prove
// it's the one who own that ip address and code for calculating the payment election.
//
class CMasternode : public CSignedMessage
{
private:
    // critical section to protect the inner data structures
    mutable RecursiveMutex cs;
    bool fCollateralSpent{false};

public:
    enum state {
        MASTERNODE_PRE_ENABLED,
        MASTERNODE_ENABLED,
        MASTERNODE_EXPIRED,
        MASTERNODE_REMOVE,
        MASTERNODE_VIN_SPENT,
    };

    CTxIn vin;
    CService addr;
    CPubKey pubKeyCollateralAddress;
    CPubKey pubKeyMasternode;
    int64_t sigTime; //mnb message time
    int protocolVersion;
    int nScanningErrorCount;
    int nLastScanningErrorBlockHeight;
    CMasternodePing lastPing;

    explicit CMasternode();
    CMasternode(const CMasternode& other);

    // override CSignedMessage functions
    uint256 GetSignatureHash() const override;
    std::string GetStrMessage() const override;
    const CTxIn GetVin() const { return vin; };

    void SetLastPing(const CMasternodePing& _lastPing) { WITH_LOCK(cs, lastPing = _lastPing;); }

    CMasternode& operator=(const CMasternode& other)
    {
        nMessVersion = other.nMessVersion;
        vchSig = other.vchSig;
        vin = other.vin;
        addr = other.addr;
        pubKeyCollateralAddress = other.pubKeyCollateralAddress;
        pubKeyMasternode = other.pubKeyMasternode;
        sigTime = other.sigTime;
        lastPing = other.lastPing;
        protocolVersion = other.protocolVersion;
        nScanningErrorCount = other.nScanningErrorCount;
        nLastScanningErrorBlockHeight = other.nLastScanningErrorBlockHeight;
        return *this;
    }

    friend bool operator==(const CMasternode& a, const CMasternode& b)
    {
        return a.vin == b.vin;
    }
    friend bool operator!=(const CMasternode& a, const CMasternode& b)
    {
        return !(a.vin == b.vin);
    }

    uint256 CalculateScore(const uint256& hash) const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        LOCK(cs);

        READWRITE(vin);
        READWRITE(addr);
        READWRITE(pubKeyCollateralAddress);
        READWRITE(pubKeyMasternode);
        READWRITE(vchSig);
        READWRITE(sigTime);
        READWRITE(protocolVersion);
        READWRITE(lastPing);
        READWRITE(nScanningErrorCount);
        READWRITE(nLastScanningErrorBlockHeight);
    }

    template <typename Stream>
    CMasternode(deserialize_type, Stream& s)
    {
        Unserialize(s);
    }

    bool UpdateFromNewBroadcast(CMasternodeBroadcast& mnb);

    CMasternode::state GetActiveState() const;

    bool IsBroadcastedWithin(int seconds)
    {
        return (GetAdjustedTime() - sigTime) < seconds;
    }

    bool IsPingedWithin(int seconds, int64_t now = -1) const
    {
        now == -1 ? now = GetAdjustedTime() : now;
        return lastPing.IsNull() ? false : now - lastPing.sigTime < seconds;
    }

    void SetSpent() { fCollateralSpent = true; }

    void Disable()
    {
        LOCK(cs);
        sigTime = 0;
        lastPing = CMasternodePing();
    }

    bool IsEnabled() const
    {
        return GetActiveState() == MASTERNODE_ENABLED;
    }

    bool IsPreEnabled() const
    {
        return GetActiveState() == MASTERNODE_PRE_ENABLED;
    }

    bool IsAvailableState() const
    {
        state s = GetActiveState();
        return s == MASTERNODE_ENABLED || s == MASTERNODE_PRE_ENABLED;
    }

    std::string Status() const
    {
        auto activeState = GetActiveState();
        if (activeState == CMasternode::MASTERNODE_PRE_ENABLED) return "PRE_ENABLED";
        if (activeState == CMasternode::MASTERNODE_ENABLED) return "ENABLED";
        if (activeState == CMasternode::MASTERNODE_EXPIRED) return "EXPIRED";
        if (activeState == CMasternode::MASTERNODE_VIN_SPENT) return "VIN_SPENT";
        if (activeState == CMasternode::MASTERNODE_REMOVE) return "REMOVE";
        return strprintf("INVALID_%d", activeState);
    }

    bool IsValidNetAddr() const;

    /// Is the input associated with collateral public key? (and there is 10000 IDC - checking if valid masternode)
    bool IsInputAssociatedWithPubkey() const;
};


//
// The Masternode Broadcast Class : Contains a different serialize method for sending masternodes through the network
//

class CMasternodeBroadcast : public CMasternode
{
public:
    CMasternodeBroadcast();
    CMasternodeBroadcast(CService newAddr, CTxIn newVin, CPubKey newPubkey, CPubKey newPubkey2, int protocolVersionIn, const CMasternodePing& _lastPing);
    CMasternodeBroadcast(const CMasternode& mn);

    bool CheckAndUpdate(int& nDoS);
    bool CheckInputsAndAdd(int chainHeight, int& nDos);

    uint256 GetHash() const;

    void Relay();

    // special sign/verify
    bool Sign(const CKey& key, const CPubKey& pubKey);
    bool Sign(const std::string strSignKey);
    bool CheckSignature() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(vin);
        READWRITE(addr);
        READWRITE(pubKeyCollateralAddress);
        READWRITE(pubKeyMasternode);
        READWRITE(vchSig);
        READWRITE(sigTime);
        READWRITE(protocolVersion);
        READWRITE(lastPing);
        READWRITE(nMessVersion);
    }

    /// Create Masternode broadcast, needs to be relayed manually after that
    static bool Create(const CTxIn& vin, const CService& service, const CKey& keyCollateralAddressNew, const CPubKey& pubKeyCollateralAddressNew, const CKey& keyMasternodeNew, const CPubKey& pubKeyMasternodeNew, std::string& strErrorRet, CMasternodeBroadcast& mnbRet);
    static bool Create(const std::string& strService, const std::string& strKey, const std::string& strTxHash, const std::string& strOutputIndex, std::string& strErrorRet, CMasternodeBroadcast& mnbRet, bool fOffline = false);
    static bool CheckDefaultPort(CService service, std::string& strErrorRet, const std::string& strContext);
};

#endif
