// Copyright (c) 2011-2013 The PPCoin developers
// Copyright (c) 2013-2014 The NovaCoin Developers
// Copyright (c) 2014-2018 The BlackCoin Developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ISLAMIC_DIGITAL_COIN_LEGACY_MODIFIER_H
#define ISLAMIC_DIGITAL_COIN_LEGACY_MODIFIER_H

#include "chain.h"
#include "stakeinput.h"

// Old Modifier - Only for IBD
bool GetOldStakeModifier(CStakeInput* stake, uint64_t& nStakeModifier);
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier);

#endif // ISLAMIC_DIGITAL_COIN_LEGACY_MODIFIER_H
