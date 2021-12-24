// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpc/client.h"

#include <set>
#include <stdint.h>

#include <univalue.h>


class CRPCConvertParam
{
public:
    std::string methodName; //! method whose params want conversion
    int paramIdx;           //! 0-based idx of param to convert
};
// ***TODO***
static const CRPCConvertParam vRPCConvertParams[] = {
    { "stop", 0 },
    { "setmocktime", 0 },
    { "getaddednodeinfo", 0 },
    { "setgenerate", 0 },
    { "setgenerate", 1 },
    { "generate", 0 },
    { "getnetworkhashps", 0 },
    { "getnetworkhashps", 1 },
    { "delegatestake", 1 },
    { "delegatestake", 3 },
    { "delegatestake", 4 },
    { "delegatestake", 5 },
    { "rawdelegatestake", 1 },
    { "rawdelegatestake", 3 },
    { "rawdelegatestake", 4 },
    { "sendtoaddress", 1 },
    { "settxfee", 0 },
    { "getreceivedbyaddress", 1 },
    { "listreceivedbyshieldaddress", 1 },
    { "getreceivedbylabel", 1 },
    { "listcoldutxos", 0 },
    { "listdelegators", 0 },
    { "getsaplingnotescount", 0 },
    { "listreceivedbyaddress", 0 },
    { "listreceivedbyaddress", 1 },
    { "listreceivedbyaddress", 2 },
    { "listreceivedbylabel", 0 },
    { "listreceivedbylabel", 1 },
    { "listreceivedbylabel", 2 },
    { "getbalance", 0 },
    { "getbalance", 1 },
    { "getbalance", 2 },
    { "getbalance", 3 },
    { "getshieldbalance", 1 },
    { "getshieldbalance", 2 },
    { "rawshieldsendmany", 1 },
    { "rawshieldsendmany", 2 },
    { "rawshieldsendmany", 3 },
    { "shieldsendmany", 1 },
    { "shieldsendmany", 2 },
    { "shieldsendmany", 3 },
    { "getblockhash", 0 },
    { "waitforblockheight", 0 },
    { "waitforblockheight", 1 },
    { "waitforblock", 1 },
    { "waitforblock", 2 },
    { "waitfornewblock", 0 },
    { "waitfornewblock", 1 },
    { "listtransactions", 1 },
    { "listtransactions", 2 },
    { "listtransactions", 3 },
    { "listtransactions", 4 },
    { "listtransactions", 5 },
    { "walletpassphrase", 1 },
    { "walletpassphrase", 2 },
    { "getblocktemplate", 0 },
    { "listsinceblock", 1 },
    { "listsinceblock", 2 },
    { "sendmany", 1 },
    { "sendmany", 2 },
    { "addmultisigaddress", 0 },
    { "addmultisigaddress", 1 },
    { "createmultisig", 0 },
    { "createmultisig", 1 },
    { "listunspent", 0 },
    { "listunspent", 1 },
    { "listunspent", 2 },
    { "listunspent", 3 },
    { "listshieldunspent", 0 },
    { "listshieldunspent", 1 },
    { "listshieldunspent", 2 },
    { "listshieldunspent", 3 },
    { "logging", 0 },
    { "logging", 1 },
    { "getblock", 1 },
    { "getblockheader", 1 },
    { "gettransaction", 1 },
    { "getrawtransaction", 1 },
    { "createrawtransaction", 0 },
    { "createrawtransaction", 1 },
    { "createrawtransaction", 2 },
    { "fundrawtransaction", 1 },
    { "signrawtransaction", 1 },
    { "signrawtransaction", 2 },
    { "sendrawtransaction", 1 },
    { "sethdseed", 0 },
    { "gettxout", 1 },
    { "gettxout", 2 },
    { "lockunspent", 0 },
    { "lockunspent", 1 },
    { "importprivkey", 2 },
    { "importprivkey", 3 },
    { "importaddress", 2 },
    { "importaddress", 3 },
    { "importpubkey", 2 },
    { "exportsaplingkey", 1 },
    { "importsaplingkey", 2 },
    { "importsaplingviewingkey", 2 },
    { "verifychain", 0 },
    { "verifychain", 1 },
    { "keypoolrefill", 0 },
    { "getrawmempool", 0 },
    { "estimatefee", 0 },
    { "estimatesmartfee", 0 },
    { "prioritisetransaction", 1 },
    { "prioritisetransaction", 2 },
    { "setban", 2 },
    { "setban", 3 },
    { "spork", 1 },
    { "preparebudget", 2 },
    { "preparebudget", 3 },
    { "preparebudget", 5 },
    { "submitbudget", 2 },
    { "submitbudget", 3 },
    { "submitbudget", 5 },
    { "submitbudget", 7 },
    // TODO: remove this and switch over to proper arg parsing in rpc/masternode.cpp for the second argument
    //{"startmasternode", 1},
    { "startmasternode", 3 },
    { "mnvoteraw", 1 },
    { "mnvoteraw", 4 },
    { "setstakesplitthreshold", 0 },
    { "autocombinerewards", 0 },
    { "autocombinerewards", 1 },
    { "getblockindexstats", 0 },
    { "getblockindexstats", 1 },
    { "getfeeinfo", 0 },
    { "getsupplyinfo", 0 },
};

class CRPCConvertTable
{
private:
    std::set<std::pair<std::string, int> > members;

public:
    CRPCConvertTable();

    bool convert(const std::string& method, int idx)
    {
        return (members.count(std::make_pair(method, idx)) > 0);
    }
};

CRPCConvertTable::CRPCConvertTable()
{
    const unsigned int n_elem =
        (sizeof(vRPCConvertParams) / sizeof(vRPCConvertParams[0]));

    for (unsigned int i = 0; i < n_elem; i++) {
        members.emplace(vRPCConvertParams[i].methodName, vRPCConvertParams[i].paramIdx);
    }
}

static CRPCConvertTable rpcCvtTable;

/** Non-RFC4627 JSON parser, accepts internal values (such as numbers, true, false, null)
 * as well as objects and arrays.
 */
UniValue ParseNonRFCJSONValue(const std::string& strVal)
{
    UniValue jVal;
    if (!jVal.read(std::string("[")+strVal+std::string("]")) ||
        !jVal.isArray() || jVal.size()!=1)
        throw std::runtime_error(std::string("Error parsing JSON:")+strVal);
    return jVal[0];
}

/** Convert strings to command-specific RPC representation */
UniValue RPCConvertValues(const std::string &strMethod, const std::vector<std::string> &strParams)
{
    UniValue params(UniValue::VARR);

    for (unsigned int idx = 0; idx < strParams.size(); idx++) {
        const std::string& strVal = strParams[idx];

        if (!rpcCvtTable.convert(strMethod, idx)) {
            // insert string value directly
            params.push_back(strVal);
        } else {
            // parse string as JSON, insert bool/number/object/etc. value
            params.push_back(ParseNonRFCJSONValue(strVal));
        }
    }

    return params;
}
