/*
Tencent is pleased to support the open source community by making 
PhxPaxos available.
Copyright (C) 2016 THL A29 Limited, a Tencent company. 
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may 
not use this file except in compliance with the License. You may 
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" basis, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
implied. See the License for the specific language governing 
permissions and limitations under the License.

See the AUTHORS file for names of contributors. 
*/

#include "kv_paxos.h"
#include <assert.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace phxpaxos;
using namespace std;

namespace phxkv
{

PhxKV :: PhxKV(const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList,
        const std::string & sKVDBPath, const std::string & sPaxosLogPath)
    : m_oMyNode(oMyNode), m_vecNodeList(vecNodeList), 
    m_sKVDBPath(sKVDBPath), m_sPaxosLogPath(sPaxosLogPath), 
    m_poPaxosNode(nullptr), m_oPhxKVSM(sKVDBPath)
{
    //only show you how to use multi paxos group, you can set as 1, 2, or any other number.
    //not too large.
    m_iGroupCount = 3;
}

PhxKV :: ~PhxKV()
{
    delete m_poPaxosNode;
}

const phxpaxos::NodeInfo PhxKV :: GetMaster(const std::string & sKey)
{
    int iGroupIdx = GetGroupIdx(sKey);
    return m_poPaxosNode->GetMaster(iGroupIdx);
}

const bool PhxKV :: IsIMMaster(const std::string & sKey)
{
    int iGroupIdx = GetGroupIdx(sKey);
    return m_poPaxosNode->IsIMMaster(iGroupIdx);
}

int PhxKV :: RunPaxos()
{
    bool bSucc = m_oPhxKVSM.Init();
    if (!bSucc)
    {
        return -1;
    }
    
    Options oOptions;

    oOptions.sLogStoragePath = m_sPaxosLogPath;

    //this groupcount means run paxos group count.
    //every paxos group is independent, there are no any communicate between any 2 paxos group.
    oOptions.iGroupCount = m_iGroupCount;

    oOptions.oMyNode = m_oMyNode;
    oOptions.vecNodeInfoList = m_vecNodeList;

    //because all group share state machine(kv), so every group have same sate machine.
    //just for split key to different paxos group, to upgrate performance.
    for (int iGroupIdx = 0; iGroupIdx < m_iGroupCount; iGroupIdx++)
    {
        GroupSMInfo oSMInfo;
        oSMInfo.iGroupIdx = iGroupIdx;
        oSMInfo.vecSMList.push_back(&m_oPhxKVSM);
        oSMInfo.bIsUseMaster = true;

        oOptions.vecGroupSMInfoList.push_back(oSMInfo);
    }

    //set logfunc
    oOptions.pLogFunc = LOGGER->GetLogFunc();

    int ret = Node::RunNode(oOptions, m_poPaxosNode);
    if (ret != 0)
    {
        PLErr("run paxos fail, ret %d", ret);
        return ret;
    }

    PLImp("run paxos ok\n");
    return 0;
}

int PhxKV :: GetGroupIdx(const std::string & sKey)
{
    uint32_t iHashNum = 0;
    for (size_t i = 0; i < sKey.size(); i++)
    {
        iHashNum = iHashNum * 7 + ((int)sKey[i]);
    }

    return iHashNum % m_iGroupCount;
}

int PhxKV :: KVPropose(const std::string & sKey, const std::string & sPaxosValue, PhxKVSMCtx & oPhxKVSMCtx)
{
    int iGroupIdx = GetGroupIdx(sKey);

    SMCtx oCtx;
    //smid must same to PhxKVSM.SMID().
    oCtx.m_iSMID = 1;
    oCtx.m_pCtx = (void *)&oPhxKVSMCtx;

    uint64_t llInstanceID = 0;
    int ret = m_poPaxosNode->Propose(iGroupIdx, sPaxosValue, llInstanceID, &oCtx);
    if (ret != 0)
    {
        PLErr("paxos propose fail, key %s groupidx %d ret %d", iGroupIdx, ret);
        return ret;
    }

    return 0;
}

PhxKVStatus PhxKV :: Put(
        const std::string & sKey, 
        const std::string & sValue, 
        const uint64_t llVersion)
{
    string sPaxosValue;
    bool bSucc = PhxKVSM::MakeSetOpValue(sKey, sValue, llVersion, sPaxosValue);
    if (!bSucc)
    {
        return PhxKVStatus::FAIL;
    }

    PhxKVSMCtx oPhxKVSMCtx;
    int ret = KVPropose(sKey, sPaxosValue, oPhxKVSMCtx);
    if (ret != 0)
    {
        return PhxKVStatus::FAIL;
    }

    if (oPhxKVSMCtx.iExecuteRet == KVCLIENT_OK)
    {
        return PhxKVStatus::SUCC;
    }
    else if (oPhxKVSMCtx.iExecuteRet == KVCLIENT_KEY_VERSION_CONFLICT)
    {
        return PhxKVStatus::VERSION_CONFLICT; 
    }
    else
    {
        return PhxKVStatus::FAIL;
    }
}

PhxKVStatus PhxKV :: GetLocal(
        const std::string & sKey, 
        std::string & sValue, 
        uint64_t & llVersion)
{
    int ret = m_oPhxKVSM.GetKVClient()->Get(sKey, sValue, llVersion);
    if (ret == KVCLIENT_OK)
    {
        return PhxKVStatus::SUCC;
    }
    else if (ret == KVCLIENT_KEY_NOTEXIST)
    {
        return PhxKVStatus::KEY_NOTEXIST; 
    }
    else
    {
        return PhxKVStatus::FAIL;
    }
}

PhxKVStatus PhxKV :: Delete( 
        const std::string & sKey, 
        const uint64_t llVersion)
{
    string sPaxosValue;
    bool bSucc = PhxKVSM::MakeDelOpValue(sKey, llVersion, sPaxosValue);
    if (!bSucc)
    {
        return PhxKVStatus::FAIL;
    }

    PhxKVSMCtx oPhxKVSMCtx;
    int ret = KVPropose(sKey, sPaxosValue, oPhxKVSMCtx);
    if (ret != 0)
    {
        return PhxKVStatus::FAIL;
    }

    if (oPhxKVSMCtx.iExecuteRet == KVCLIENT_OK)
    {
        return PhxKVStatus::SUCC;
    }
    else if (oPhxKVSMCtx.iExecuteRet == KVCLIENT_KEY_VERSION_CONFLICT)
    {
        return PhxKVStatus::VERSION_CONFLICT; 
    }
    else
    {
        return PhxKVStatus::FAIL;
    }
}

}

