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

#include "bench_server.h"
#include <assert.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace phxpaxos;
using namespace std;

namespace bench
{

BenchServer :: BenchServer(const int iGroupCount, const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList)
    : m_oMyNode(oMyNode), m_vecNodeList(vecNodeList), m_poPaxosNode(nullptr)
{
    m_iGroupCount = iGroupCount;

    for (int iGroupIdx = 0; iGroupIdx < m_iGroupCount; iGroupIdx++)
    {
        BenchSM * poBenchSM = new BenchSM(oMyNode.GetNodeID(), iGroupIdx);
        assert(poBenchSM != nullptr);
        m_vecSMList.push_back(poBenchSM);
    }
}

BenchServer :: ~BenchServer()
{
    delete m_poPaxosNode;

    for (auto & poBenchSM : m_vecSMList)
    {
        delete poBenchSM;
    }
}

int BenchServer :: MakeLogStoragePath(std::string & sLogStoragePath)
{
    char sTmp[128] = {0};
    snprintf(sTmp, sizeof(sTmp), "./logpath_%s_%d", m_oMyNode.GetIP().c_str(), m_oMyNode.GetPort());

    sLogStoragePath = string(sTmp);

    if (access(sLogStoragePath.c_str(), F_OK) == -1)
    {
        if (mkdir(sLogStoragePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
        {       
            printf("Create dir fail, path %s\n", sLogStoragePath.c_str());
            return -1;
        }       
    }

    return 0;
}

int BenchServer :: RunPaxos()
{
    Options oOptions;

    int ret = MakeLogStoragePath(oOptions.sLogStoragePath);
    if (ret != 0)
    {
        return ret;
    }

    //this groupcount means run paxos group count.
    //every paxos group is independent, there are no any communicate between any 2 paxos group.
    oOptions.iGroupCount = m_iGroupCount;

    oOptions.oMyNode = m_oMyNode;
    oOptions.vecNodeInfoList = m_vecNodeList;

    //different paxos group can have different state machine.
    for (auto & poBenchSM : m_vecSMList)
    {
        GroupSMInfo oSMInfo;
        oSMInfo.iGroupIdx = poBenchSM->GetGroupIdx();
        //one paxos group can have multi state machine.
        oSMInfo.vecSMList.push_back(poBenchSM);

        oOptions.vecGroupSMInfoList.push_back(oSMInfo);
    }

    //oOptions.eLogLevel = LogLevel::LogLevel_Error;
    oOptions.bUseBatchPropose = true;

    ret = Node::RunNode(oOptions, m_poPaxosNode);
    if (ret != 0)
    {
        printf("run paxos fail, ret %d\n", ret);
        return ret;
    }

    for (int iGroupIdx = 0; iGroupIdx < m_iGroupCount; iGroupIdx++)
    {
        m_poPaxosNode->SetBatchDelayTimeMs(iGroupIdx, 10);
        m_poPaxosNode->SetBatchCount(iGroupIdx, 10);
    }

    printf("run paxos ok\n");
    return 0;
}

int BenchServer :: ReadyBench()
{
    int ret = 0;
    for (int i = 0; i < m_iGroupCount; i++)
    {
        ret = Write(i, "start bench");
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

int BenchServer :: Write(const std::string & sBenchValue)
{
    int iGroupIdx = rand() % m_iGroupCount;
    return Write(iGroupIdx, sBenchValue);
}

int BenchServer :: Write(const int iGroupIdx, const std::string & sBenchValue)
{
    
    SMCtx oCtx;
    //smid must same to BenchSM.SMID().
    oCtx.m_iSMID = 1;
    oCtx.m_pCtx = nullptr;

    uint64_t llInstanceID = 0;
    int ret = m_poPaxosNode->Propose(iGroupIdx, sBenchValue, llInstanceID, &oCtx);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}
    
}


