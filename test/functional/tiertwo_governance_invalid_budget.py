#!/usr/bin/env python3
# Copyright (c) 2021 The PIVX developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import islamic_digital_coinTestFramework
from test_framework.util import (
    assert_equal,
    p2p_port,
)

import os
import time

class GovernanceInvalidBudgetTest(islamic_digital_coinTestFramework):

    def set_test_params(self):
        self.setup_clean_chain = True
        # 4 nodes:
        # - 1 miner/mncontroller
        # - 2 remote mns
        # - 1 other node to stake a forked chain
        self.num_nodes = 4
        self.extra_args = [["-sporkkey=932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi"],
                           [],
                           ["-listen", "-externalip=127.0.0.1"],
                           ["-listen", "-externalip=127.0.0.1"],
                           ]
        self.enable_mocktime()

        self.minerAPos = 0
        self.minerBPos = 1
        self.remoteOnePos = 1
        self.remoteTwoPos = 2

        self.masternodeOneAlias = "mnOne"
        self.masternodeTwoAlias = "mntwo"

        self.mnOnePrivkey = "9247iC59poZmqBYt9iDh9wDam6v9S1rW5XekjLGyPnDhrDkP4AK"
        self.mnTwoPrivkey = "92Hkebp3RHdDidGZ7ARgS4orxJAGyFUPDXNqtsYsiwho1HGVRbF"

    def run_test(self):
        self.minerA = self.nodes[self.minerAPos]     # also controller of mn1 and mn2
        self.minerB = self.nodes[self.minerBPos]
        self.mn1 = self.nodes[self.remoteOnePos]
        self.mn2 = self.nodes[self.remoteTwoPos]
        self.setupContext()

        # Create a proposal and vote on it
        next_superblock = self.minerA.getnextsuperblock()
        payee = self.minerA.getnewaddress()
        self.log.info("Creating a proposal to be paid at block %d" % next_superblock)
        proposalFeeTxId = self.minerA.preparebudget("test1", "https://test1.org", 2,
                                               next_superblock, payee, 300)
        self.stake_and_ping(self.minerAPos, 3, [self.mn1, self.mn2])
        proposalHash = self.minerA.submitbudget("test1", "https://test1.org", 2,
                                           next_superblock, payee, 300, proposalFeeTxId)
        time.sleep(1)
        self.stake_and_ping(self.minerAPos, 7, [self.mn1, self.mn2])
        self.log.info("Vote for the proposal and check projection...")
        self.minerA.mnbudgetvote("alias", proposalHash, "yes", self.masternodeOneAlias)
        self.minerA.mnbudgetvote("alias", proposalHash, "yes", self.masternodeTwoAlias)
        time.sleep(1)
        self.stake_and_ping(self.minerAPos, 1, [self.mn1, self.mn2])
        projection = self.minerB.getbudgetprojection()[0]
        assert_equal(projection["Name"], "test1")
        assert_equal(projection["Hash"], proposalHash)
        assert_equal(projection["Yeas"], 2)

        # Create invalid finalized budget and vote on it
        self.log.info("Creating invalid budget finalization...")
        self.stake_and_ping(self.minerAPos, 5, [self.mn1, self.mn2])

        budgetname = "invalid finalization"
        blockstart = self.minerA.getnextsuperblock()
        proposals = []
        badPropId = "aa0061d705de36385c37701e7632408bd9d2876626b1299a17f7dc818c0ad285"
        badPropPayee = "8c988f1a4a4de2161e0f50aac7f17e7f9555caa4"
        badPropAmount = 500
        proposals.append({"proposalid": badPropId, "payee": badPropPayee, "amount": badPropAmount})
        res = self.minerA.createrawmnfinalbudget(budgetname, blockstart, proposals)
        assert(res["result"] == "tx_fee_sent")
        feeBudgetId = res["id"]
        time.sleep(1)
        self.stake_and_ping(self.minerAPos, 4, [self.mn1, self.mn2])
        res = self.minerA.createrawmnfinalbudget(budgetname, blockstart, proposals, feeBudgetId)
        assert(res["result"] == "error") # not accepted

        self.log.info("Good, invalid budget not accepted.")

        # Stake up until the block before the superblock.
        skip_blocks = next_superblock - self.minerA.getblockcount() - 1
        self.stake_and_ping(self.minerAPos, skip_blocks, [self.mn1, self.mn2])

        # mine the superblock and check payment, it must not pay to the invalid finalization.
        self.log.info("Checking superblock...")
        self.stake_and_ping(self.nodes.index(self.minerA), 1, [])
        assert_equal(self.minerA.getblockcount(), next_superblock)
        coinstake = self.minerA.getrawtransaction(self.minerA.getblock(self.minerA.getbestblockhash())["tx"][1], True)
        budget_payment_out = coinstake["vout"][-1]
        assert(budget_payment_out["scriptPubKey"]["hex"] != badPropPayee)
        assert(budget_payment_out["value"] != badPropAmount)

        self.log.info("All good.")

    def send_3_pings(self, mn_list):
        self.advance_mocktime(30)
        self.send_pings(mn_list)
        self.stake_and_ping(self.minerAPos, 1, mn_list)
        self.advance_mocktime(30)
        self.send_pings(mn_list)
        time.sleep(2)

    def setupContext(self):
        # First mine 250 PoW blocks (50 with minerB, 200 with minerA)
        self.log.info("Generating 259 blocks...")
        for _ in range(2):
            for _ in range(25):
                self.mocktime = self.generate_pow(self.minerBPos, self.mocktime)
            self.sync_blocks()
            for _ in range(100):
                self.mocktime = self.generate_pow(self.minerAPos, self.mocktime)
            self.sync_blocks()
        # Then stake 9 blocks with minerA
        self.stake_and_ping(self.minerAPos, 9, [])
        for n in self.nodes:
            assert_equal(n.getblockcount(), 259)

        # Setup Masternodes
        self.log.info("Masternodes setup...")
        ownerdir = os.path.join(self.options.tmpdir, "node%d" % self.minerAPos, "regtest")
        self.mnOneCollateral = self.setupMasternode(self.minerA, self.minerA, self.masternodeOneAlias,
                                                    ownerdir, self.remoteOnePos, self.mnOnePrivkey)
        self.mnTwoCollateral = self.setupMasternode(self.minerA, self.minerA, self.masternodeTwoAlias,
                                                    ownerdir, self.remoteTwoPos, self.mnTwoPrivkey)

        # Activate masternodes
        self.log.info("Masternodes activation...")
        self.stake_and_ping(self.minerAPos, 1, [])
        time.sleep(3)
        self.advance_mocktime(10)
        remoteOnePort = p2p_port(self.remoteOnePos)
        remoteTwoPort = p2p_port(self.remoteTwoPos)
        self.mn1.initmasternode(self.mnOnePrivkey, "127.0.0.1:"+str(remoteOnePort))
        self.mn2.initmasternode(self.mnTwoPrivkey, "127.0.0.1:"+str(remoteTwoPort))
        self.stake_and_ping(self.minerAPos, 1, [])
        self.wait_until_mnsync_finished()
        self.controller_start_masternode(self.minerA, self.masternodeOneAlias)
        self.controller_start_masternode(self.minerA, self.masternodeTwoAlias)
        self.wait_until_mn_preenabled(self.mnOneCollateral, 40)
        self.wait_until_mn_preenabled(self.mnOneCollateral, 40)
        self.send_3_pings([self.mn1, self.mn2])
        self.wait_until_mn_enabled(self.mnOneCollateral, 120, [self.mn1, self.mn2])
        self.wait_until_mn_enabled(self.mnOneCollateral, 120, [self.mn1, self.mn2])

        # activate sporks
        self.log.info("Masternodes enabled. Activating sporks.")
        self.activate_spork(self.minerAPos, "SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT")
        self.activate_spork(self.minerAPos, "SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT")
        self.activate_spork(self.minerAPos, "SPORK_13_ENABLE_SUPERBLOCKS")


if __name__ == '__main__':
    GovernanceInvalidBudgetTest().main()