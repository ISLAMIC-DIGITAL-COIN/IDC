// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "chainparamsseeds.h"
#include "consensus/merkle.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

#include <assert.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.vtx.push_back(std::make_shared<const CTransaction>(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.nVersion = nVersion;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

void CChainParams::UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    assert(IsRegTestNet()); // only available for regtest
    assert(idx > Consensus::BASE_NETWORK && idx < Consensus::MAX_NETWORK_UPGRADES);
    consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
}

/**
 * Build the genesis block. Note that the output of the genesis coinbase cannot
 * be spent as it did not originally exist in the database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Ethereum remains the most popular platform for Metaverse coins";
    const CScript genesisOutputScript = CScript() << ParseHex("04c10e38b2713ccf322a7dad62da5815ab7c10bd025814ce121ba72607c572b7710c02b05b2acd05b4ceb9c4b77b26d92428c61356cd42774abbea0a073b2de0c4") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static Checkpoints::MapCheckpoints mapCheckpoints = {
    {0, uint256S("000005eac93c81de45dd620c8cd7d82aeeee8ff9e2c9b83da40ffd52e644d6f3")},
    {1, uint256S("00000bbfd9d18284263cdce11a208cb8710de48f6324f1eda237e868f4e0dd51")},
    {487, uint256S("0000004900fea382f186ea5115393a9507c8d8325bd52cf9cbc5e1a89502bdc5")},
    {1946, uint256S("ae3c9a49a922d497701e9dd68eeaa615491bc27a921fa3b8d1b995476e6f9685")},
    {3208, uint256S("78dc0434d1c039f2066669c1a5c5781204dd5c353a3acb388487f86e7e94e11b")},
    {4526, uint256S("ddff4eda1681fe6ab6ee9e882f5cc30ef4e65b50dbe23a8f46735fa919c4418b")},
    {5836, uint256S("1c97547bb7c5b2ca4707ef5cedb488ea3c4c7c93e2b4298f5dfd76ca4a50e411")},
    {6479, uint256S("b120173ad8b41708d0dcd9f55e2fca37d2dbb3179093c5fe7518d1fe30234703")},
    {7543, uint256S("6dd71b92f33b758ce295f93e90043248a7da4fc0fc80e91623b998c4221c04ad")},
	
};

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1640337210, // * UNIX timestamp of last checkpoint block
    14598,    // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    3000        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet = {
    {0, uint256S("0x000005eac93c81de45dd620c8cd7d82aeeee8ff9e2c9b83da40ffd52e644d6f3")},

};

static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1627545600,
    0,
    3000};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest = {{0, uint256S("0x001")}};
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        strNetworkID = "main";

        genesis = CreateGenesisBlock(1639386000, 393309, 0x1e0ffff0, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000005eac93c81de45dd620c8cd7d82aeeee8ff9e2c9b83da40ffd52e644d6f3"));
        assert(genesis.hashMerkleRoot == uint256S("0x86b9d659dac88dd325a6299e772b9ec75678d04b0cbb1d8d9ec5be69303e361d")); 

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.powLimit   = ~UINT256_ZERO >> 20;   // ISLAMIC_DIGITAL_COIN starting difficulty is 1 / 2^12
        consensus.posLimitV1 = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nBudgetCycleBlocks = 43200;       // approx. 1 every 30 days
        consensus.nBudgetFeeConfirmations = 6;      // Number of confirmations for the finalization fee
        consensus.nCoinbaseMaturity = 60;
        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nMasternodeCountDrift = 20;       // num of MN we allow the see-saw payments to be off by
        consensus.nMaxMoneyOut = 100000000000 * COIN;
        consensus.nPoolMaxTransactions = 3;
        consensus.nProposalEstablishmentTime = 60 * 60 * 24;    // must be at least a day old to make it into a budget
        consensus.nStakeMinAge = 60 * 60;
        consensus.nStakeMinDepth = 60;
        consensus.nTargetTimespan = 40 * 60;
        consensus.nTargetTimespanV2 = 30 * 60;
        consensus.nTargetSpacing = 2 * 60;
        consensus.nTimeSlotLength = 15;
        consensus.nMaxProposalPayments = 6;


        // spork keys
        consensus.strSporkPubKey = "0415d36914b048692852527a90d4a3260646e1059609278013f1558362f5af46a67e25a92924273dd91c85699525f230eb35fe3e6efe8f25d99dc82bf0311ca766";
        consensus.strSporkPubKeyOld = "0415d36914b048692852527a90d4a3260646e1059609278013f1558362f5af46a67e25a92924273dd91c85699525f230eb35fe3e6efe8f25d99dc82bf0311ca766";
        consensus.nTime_EnforceNewSporkKey = 1639472400;    //!> Tuesday, 14 December 2021 9:00:00 AM GMT
        consensus.nTime_RejectOldSporkKey = 1639641600;     //!> Thursday, 16 December 2021 8:00:00 AM GMT


        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 1001;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 1;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         = 1200;
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 1001;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          = 1050;
        consensus.vUpgrades[Consensus::UPGRADE_V5_0].nActivationHeight          = 1300;
        consensus.vUpgrades[Consensus::UPGRADE_V5_2].nActivationHeight          = 1300;

        consensus.vUpgrades[Consensus::UPGRADE_BIP65].hashActivationBlock =
                uint256S("0x");
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].hashActivationBlock =
                uint256S("0x");
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].hashActivationBlock =
                uint256S("0x");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0x3a;
        pchMessageStart[1] = 0x7d;
        pchMessageStart[2] = 0xd3;
        pchMessageStart[3] = 0xb2;
        nDefaultPort = 17151;

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.emplace_back("51.195.113.208", "51.195.113.208", true);
        vSeeds.emplace_back("2001:41d0:700:1b48::208", "2001:41d0:700:1b48::208", true);
        vSeeds.emplace_back("51.195.113.209", "51.195.113.209", true);
        vSeeds.emplace_back("2001:41d0:700:1b48::209", "2001:41d0:700:1b48::209", true);
        vSeeds.emplace_back("51.195.113.210", "51.195.113.210", true);
        vSeeds.emplace_back("2001:41d0:700:1b48::210", "2001:41d0:700:1b48::210", true);
        vSeeds.emplace_back("51.195.113.211", "51.195.113.211", true);
        vSeeds.emplace_back("2001:41d0:700:1b48::211", "2001:41d0:700:1b48::211", true);
        vSeeds.emplace_back("51.195.113.212", "51.195.113.212", true);
        vSeeds.emplace_back("2001:41d0:700:1b48::212", "2001:41d0:700:1b48::212", true);
        vSeeds.emplace_back("158.69.96.224", "158.69.96.224", true);
        vSeeds.emplace_back("2607:5300:60:9133::224", "2607:5300:60:9133::224", true);
        vSeeds.emplace_back("158.69.96.225", "158.69.96.225", true);
        vSeeds.emplace_back("2607:5300:60:9133::225", "2607:5300:60:9133::225", true);
        vSeeds.emplace_back("158.69.96.226", "158.69.96.226", true);
        vSeeds.emplace_back("2607:5300:60:9133::226", "2607:5300:60:9133::226", true);
        vSeeds.emplace_back("158.69.96.227", "158.69.96.227", true);
        vSeeds.emplace_back("2607:5300:60:9133::227", "2607:5300:60:9133::227", true);
        vSeeds.emplace_back("158.69.96.228", "158.69.96.228", true);
        vSeeds.emplace_back("2607:5300:60:9133::228", "2607:5300:60:9133::228", true);
        vSeeds.emplace_back("158.69.96.229", "158.69.96.229", true);
        vSeeds.emplace_back("2607:5300:60:9133::229", "2607:5300:60:9133::229", true);
        vSeeds.emplace_back("158.69.96.230", "158.69.96.230", true);
        vSeeds.emplace_back("2607:5300:60:9133::230", "2607:5300:60:9133::230", true);
        vSeeds.emplace_back("158.69.96.231", "158.69.96.231", true);
        vSeeds.emplace_back("2607:5300:60:9133::231", "2607:5300:60:9133::231", true);
        vSeeds.emplace_back("158.69.96.232", "158.69.96.232", true);
        vSeeds.emplace_back("2607:5300:60:9133::232", "2607:5300:60:9133::232", true);
        vSeeds.emplace_back("158.69.96.233", "158.69.96.233", true);
        vSeeds.emplace_back("2607:5300:60:9133::233", "2607:5300:60:9133::233", true);
        vSeeds.emplace_back("158.69.96.208", "158.69.96.208", true);
        vSeeds.emplace_back("2607:5300:60:9135::208", "2607:5300:60:9135::208", true);
        vSeeds.emplace_back("158.69.96.209", "158.69.96.209", true);
        vSeeds.emplace_back("2607:5300:60:9135::209", "2607:5300:60:9135::209", true);
        vSeeds.emplace_back("158.69.96.210", "158.69.96.210", true);
        vSeeds.emplace_back("2607:5300:60:9135::210", "2607:5300:60:9135::210", true);
        vSeeds.emplace_back("158.69.96.211", "158.69.96.211", true);
        vSeeds.emplace_back("2607:5300:60:9135::211", "2607:5300:60:9135::211", true);
        vSeeds.emplace_back("158.69.96.212", "158.69.96.212", true);
        vSeeds.emplace_back("2607:5300:60:9135::212", "2607:5300:60:9135::212", true);
        vSeeds.emplace_back("158.69.96.213", "158.69.96.213", true);
        vSeeds.emplace_back("2607:5300:60:9135::213", "2607:5300:60:9135::213", true);
        vSeeds.emplace_back("158.69.96.214", "158.69.96.214", true);
        vSeeds.emplace_back("2607:5300:60:9135::214", "2607:5300:60:9135::214", true);
        vSeeds.emplace_back("158.69.96.215", "158.69.96.215", true);
        vSeeds.emplace_back("2607:5300:60:9135::215", "2607:5300:60:9135::215", true);
        vSeeds.emplace_back("158.69.96.216", "158.69.96.216", true);
        vSeeds.emplace_back("2607:5300:60:9135::216", "2607:5300:60:9135::216", true);
        vSeeds.emplace_back("158.69.96.217", "158.69.96.217", true);
        vSeeds.emplace_back("2607:5300:60:9135::217", "2607:5300:60:9135::217", true);  



        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 103);		// starting with 'i'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 13);
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 31);     // starting with 'D'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 212);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x02, 0x2D, 0x25, 0x33};
        base58Prefixes[EXT_SECRET_KEY] = {0x02, 0x21, 0x31, 0x2B};
        // BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = {0x80, 0x00, 0x00, 0x77};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        // Sapling
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "is";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "iviews";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "idcks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "i-secret-spending-key-main";
        bech32HRPs[SAPLING_EXTENDED_FVK]         = "ixviews";
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }

};

/**
 * Testnet (v5)
 */
class CTestNetParams : public CChainParams
{
public:
    CTestNetParams()
    {
        strNetworkID = "test";

        genesis = CreateGenesisBlock(1454124731, 2402015, 0x1e0ffff0, 1, 250 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
      //  assert(consensus.hashGenesisBlock == uint256S("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
      //  assert(genesis.hashMerkleRoot == uint256S("0x1b2ef6e2f28be914103a277377ae7729dcd125dfeb8bf97bd5964ba72b6dc39b"));

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.powLimit   = ~UINT256_ZERO >> 20;   // ISLAMIC_DIGITAL_COIN starting difficulty is 1 / 2^12
        consensus.posLimitV1 = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nBudgetCycleBlocks = 144;         // approx 10 cycles per day
        consensus.nBudgetFeeConfirmations = 3;      // (only 8-blocks window for finalization on testnet)
        consensus.nCoinbaseMaturity = 60;
        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nMasternodeCountDrift = 20;       // num of MN we allow the see-saw payments to be off by
        consensus.nMaxMoneyOut = 20000000 * COIN;
        consensus.nPoolMaxTransactions = 3;
        consensus.nProposalEstablishmentTime = 60 * 60 * 24;  // at least 5 min old to make it into a budget
        consensus.nStakeMinAge = 60 * 60;
        consensus.nStakeMinDepth = 60;
        consensus.nTargetTimespan = 40 * 60;
        consensus.nTargetTimespanV2 = 30 * 60;
        consensus.nTargetSpacing = 2 * 60;
        consensus.nTimeSlotLength = 15;
        consensus.nMaxProposalPayments = 6;

        // spork keys
        consensus.strSporkPubKey = "04677c34726c491117265f4b1c83cef085684f36c8df5a97a3a42fc499316d0c4e63959c9eca0dba239d9aaaf72011afffeb3ef9f51b9017811dec686e412eb504";
        consensus.strSporkPubKeyOld = "04E88BB455E2A04E65FCC41D88CD367E9CCE1F5A409BE94D8C2B4B35D223DED9C8E2F4E061349BA3A38839282508066B6DC4DB72DD432AC4067991E6BF20176127";
        consensus.nTime_EnforceNewSporkKey = 1608512400;    //!> December 21, 2020 01:00:00 AM GMT
        consensus.nTime_RejectOldSporkKey = 1614560400;     //!> March 1, 2021 01:00:00 AM GMT


        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 1001;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 1;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         = 1300;
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 1001;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          = 1100;
        consensus.vUpgrades[Consensus::UPGRADE_V5_0].nActivationHeight          = 1300;
        consensus.vUpgrades[Consensus::UPGRADE_V5_2].nActivationHeight          = 1300;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xa4;
        pchMessageStart[1] = 0xe1;
        pchMessageStart[2] = 0xc3;
        pchMessageStart[3] = 0x5a;
        nDefaultPort = 11012;

        // nodes with support for servicebits filtering should be at the top
       /* vSeeds.emplace_back(""fuzzbawls.pw", "islamic_digital_coin-testnet.seed.fuzzbawls.pw", true);
        vSeeds.emplace_back(""fuzzbawls.pw", "islamic_digital_coin-testnet.seed.fuzzbawls.pw", true);*/

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 139); // Testnet IDC addresses start with 'x' or 'y'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 73);     // starting with 'W'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c'
        // Testnet islamic_digital_coin BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = {0x3a, 0x80, 0x61, 0xa0};
        // Testnet islamic_digital_coin BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = {0x3a, 0x80, 0x58, 0x37};
        // Testnet islamic_digital_coin BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = {0x80, 0x00, 0x00, 0x01};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        // Sapling
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "itestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "iviewtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "idcktestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "i-secret-spending-key-test";
        bech32HRPs[SAPLING_EXTENDED_FVK]         = "ixviewtestsapling";
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams
{
public:
    CRegTestParams()
    {
        strNetworkID = "regtest";

        genesis = CreateGenesisBlock(1454124731, 2402015, 0x1e0ffff0, 1, 250 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
   //     assert(consensus.hashGenesisBlock == uint256S("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
    //    assert(genesis.hashMerkleRoot == uint256S("0x1b2ef6e2f28be914103a277377ae7729dcd125dfeb8bf97bd5964ba72b6dc39b"));

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.powLimit   = ~UINT256_ZERO >> 20;   // ISLAMIC_DIGITAL_COIN starting difficulty is 1 / 2^12
        consensus.posLimitV1 = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nBudgetCycleBlocks = 144;         // approx 10 cycles per day
        consensus.nBudgetFeeConfirmations = 3;      // (only 8-blocks window for finalization on regtest)
        consensus.nCoinbaseMaturity = 100;
        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nMasternodeCountDrift = 4;        // num of MN we allow the see-saw payments to be off by
        consensus.nMaxMoneyOut = 43199500 * COIN;
        consensus.nPoolMaxTransactions = 2;
        consensus.nProposalEstablishmentTime = 60 * 5;  // at least 5 min old to make it into a budget
        consensus.nStakeMinAge = 0;
        consensus.nStakeMinDepth = 2;
        consensus.nTargetTimespan = 40 * 60;
        consensus.nTargetTimespanV2 = 30 * 60;
        consensus.nTargetSpacing = 1 * 60;
        consensus.nTimeSlotLength = 15;
        consensus.nMaxProposalPayments = 20;

        /* Spork Key for RegTest:
        WIF private key: 932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi
        private key hex: bd4960dcbd9e7f2223f24e7164ecb6f1fe96fc3a416f5d3a830ba5720c84b8ca
        Address: yCvUVd72w7xpimf981m114FSFbmAmne7j9
        */
        consensus.strSporkPubKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 251;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 251;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 251;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_V5_0].nActivationHeight          = 300;
        consensus.vUpgrades[Consensus::UPGRADE_V5_2].nActivationHeight          = 300;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nDefaultPort = 51476;

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 139); // Testnet islamic_digital_coin addresses start with 'x' or 'y'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet islamic_digital_coin script addresses start with '8' or '9'
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 73);     // starting with 'W'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet islamic_digital_coin BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = {0x3a, 0x80, 0x61, 0xa0};
        // Testnet islamic_digital_coin BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = {0x3a, 0x80, 0x58, 0x37};
        // Testnet islamic_digital_coin BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = {0x80, 0x00, 0x00, 0x01};

        // Sapling
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "itestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "iviewtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "idcktestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "i-secret-spending-key-test";
        bech32HRPs[SAPLING_EXTENDED_FVK]         = "ixviewtestsapling";
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params()
{
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    globalChainParams->UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}
