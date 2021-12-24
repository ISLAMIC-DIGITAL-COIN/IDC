// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench.h"

#include "validation.h"

namespace block_bench {
#include "bench/data/block2680960.raw.h"
}

// These are the two major time-sinks which happen after we have fully received
// a block off the wire, but before we can relay the block on to peers using
// compact block relay.

static void DeserializeBlockTest(benchmark::State& state)
{
    CDataStream stream((const char*)block_bench::block2680960,
            (const char*)&block_bench::block2680960[sizeof(block_bench::block2680960)],
            SER_NETWORK, PROTOCOL_VERSION);
    char a;
    stream.write(&a, 1); // Prevent compaction

    while (state.KeepRunning()) {
        CBlock block;
        stream >> block;
        assert(stream.Rewind(sizeof(block_bench::block2680960)));
    }
}

static void DeserializeAndCheckBlockTest(benchmark::State& state)
{
    CDataStream stream((const char*)block_bench::block2680960,
            (const char*)&block_bench::block2680960[sizeof(block_bench::block2680960)],
            SER_NETWORK, PROTOCOL_VERSION);
    char a;
    stream.write(&a, 1); // Prevent compaction

    SelectParams(CBaseChainParams::MAIN);

    while (state.KeepRunning()) {
        CBlock block; // Note that CBlock caches its checked state, so we need to recreate it here
        stream >> block;
        assert(stream.Rewind(sizeof(block_bench::block2680960)));

        CValidationState state;
        assert(CheckBlock(block, state));
    }
}

BENCHMARK(DeserializeBlockTest);
BENCHMARK(DeserializeAndCheckBlockTest);
