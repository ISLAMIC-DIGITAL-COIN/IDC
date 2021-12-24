// Copyright (c) 2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php.

#ifndef ISLAMIC_DIGITAL_COIN_SAPLING_TEST_FIXTURE_H
#define ISLAMIC_DIGITAL_COIN_SAPLING_TEST_FIXTURE_H

#include "test/test_islamic_digital_coin.h"

/**
 * Testing setup that configures a complete environment for Sapling testing.
 */
struct SaplingTestingSetup : public TestingSetup {
    SaplingTestingSetup();
    ~SaplingTestingSetup();
};


#endif //ISLAMIC_DIGITAL_COIN_SAPLING_TEST_FIXTURE_H
