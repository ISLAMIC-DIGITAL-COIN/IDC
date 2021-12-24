// Copyright (c) 2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php.

#include "destination_io.h"
#include "base58.h"
#include "sapling/key_io_sapling.h"

namespace Standard {

    std::string EncodeDestination(const CWDestination &address, const CChainParams::Base58Type addrType) {
        const CTxDestination *dest = boost::get<CTxDestination>(&address);
        if (!dest) {
            return KeyIO::EncodePaymentAddress(*boost::get<libzcash::SaplingPaymentAddress>(&address));
        }
        return EncodeDestination(*dest, addrType);
    };

    CWDestination DecodeDestination(const std::string& strAddress)
    {
        bool isStaking = false;
        return DecodeDestination(strAddress, isStaking);
    }

    CWDestination DecodeDestination(const std::string& strAddress, bool& isStaking)
    {
        bool isShielded = false;
        return DecodeDestination(strAddress, isStaking, isShielded);
    }

    // agregar isShielded
    CWDestination DecodeDestination(const std::string& strAddress, bool& isStaking, bool& isShielded)
    {
        CWDestination dest;
        CTxDestination regDest = ::DecodeDestination(strAddress, isStaking);
        if (!IsValidDestination(regDest)) {
            const auto sapDest = KeyIO::DecodeSaplingPaymentAddress(strAddress);
            if (sapDest) {
                isShielded = true;
                return *sapDest;
            }
        }
        return regDest;

    }

    bool IsValidDestination(const CWDestination& address)
    {
        // Only regular base58 addresses and shielded addresses accepted here for now
        const libzcash::SaplingPaymentAddress *dest1 = boost::get<libzcash::SaplingPaymentAddress>(&address);
        if (dest1) return true;

        const CTxDestination *dest = boost::get<CTxDestination>(&address);
        return dest && ::IsValidDestination(*dest);
    }

    const libzcash::SaplingPaymentAddress* GetShieldedDestination(const CWDestination& dest)
    {
        return boost::get<libzcash::SaplingPaymentAddress>(&dest);
    }

    const CTxDestination* GetTransparentDestination(const CWDestination& dest)
    {
        return boost::get<CTxDestination>(&dest);
    }

} // End Standard namespace

