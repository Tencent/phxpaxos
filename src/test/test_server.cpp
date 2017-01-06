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

#include "test_server.h"
#include <assert.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils_include.h"

using namespace phxpaxos;
using namespace std;

namespace phxpaxos_test
{

TestServer :: TestServer(const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList)
    : m_oMyNode(oMyNode), m_vecNodeList(vecNodeList), m_poPaxosNode(nullptr)
{
}

TestServer :: ~TestServer()
{
    printf("start end server ip %s port %d\n", m_oMyNode.GetIP().c_str(), m_oMyNode.GetPort());
    delete m_poPaxosNode;
    printf("server ip %s port %d ended\n", m_oMyNode.GetIP().c_str(), m_oMyNode.GetPort());
}

int TestServer :: MakeLogStoragePath(std::string & sLogStoragePath)
{
    char sTmp[128] = {0};
    snprintf(sTmp, sizeof(sTmp), "./logpath_%s_%d", m_oMyNode.GetIP().c_str(), m_oMyNode.GetPort());

    sLogStoragePath = string(sTmp);

    if (access(sLogStoragePath.c_str(), F_OK) != -1)
    {
        if (FileUtils :: DeleteDir(sLogStoragePath) != 0)
        {
            printf("Delete exist logstorage dir fail\n");
            return -1;
        }
    }

    if (mkdir(sLogStoragePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
    {       
        printf("Create dir fail, path %s\n", sLogStoragePath.c_str());
        return -1;
    }       

    return 0;
}

int TestServer :: RunPaxos()
{
    Options oOptions;

    int ret = MakeLogStoragePath(oOptions.sLogStoragePath);
    if (ret != 0)
    {
        return ret;
    }

    //oOptions.iSyncInterval = 1;
    oOptions.iGroupCount = 1;

    oOptions.oMyNode = m_oMyNode;
    oOptions.vecNodeInfoList = m_vecNodeList;

    oOptions.bUseMembership = true;

    GroupSMInfo oSMInfo;
    oSMInfo.iGroupIdx = 0;
    oSMInfo.vecSMList.push_back(&m_oTestSM);
    oOptions.vecGroupSMInfoList.push_back(oSMInfo);

    oOptions.bUseBatchPropose = true;
    oOptions.bOpenChangeValueBeforePropose = true;

    ret = Node::RunNode(oOptions, m_poPaxosNode);
    if (ret != 0)
    {
        printf("run paxos fail, ret %d\n", ret);
        return ret;
    }

    m_poPaxosNode->SetBatchDelayTimeMs(0, 20);
    m_poPaxosNode->SetBatchCount(0, 10);

    printf("run paxos ok, ip %s port %d\n", m_oMyNode.GetIP().c_str(), m_oMyNode.GetPort());
    return 0;
}

int TestServer :: Write(const std::string & sTestValue, uint64_t & llInstanceID)
{
    SMCtx oCtx;
    oCtx.m_iSMID = 1;
    oCtx.m_pCtx = nullptr;

    string sPackValue = TestSM::PackTestValue(sTestValue);

    int ret = m_poPaxosNode->Propose(0, sPackValue, llInstanceID, &oCtx);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int TestServer :: BatchWrite(const std::string & sTestValue, uint64_t & llInstanceID, uint32_t & iBatchIndex)
{
    SMCtx oCtx;
    oCtx.m_iSMID = 1;
    oCtx.m_pCtx = nullptr;

    string sPackValue = TestSM::PackTestValue(sTestValue);

    int ret = m_poPaxosNode->BatchPropose(0, sPackValue, llInstanceID, iBatchIndex, &oCtx);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int TestServer :: Ready()
{
    uint64_t llInstanceID = 0;
    int ret = m_poPaxosNode->Propose(0, "nullvalue", llInstanceID);
    if (ret == 0)
    {
        printf("ready paxos ok, ip %s port %d\n", m_oMyNode.GetIP().c_str(), m_oMyNode.GetPort());
    }
    else 
    {
        printf("ready paxos fail, ip %s port %d ret %d\n", 
                m_oMyNode.GetIP().c_str(), m_oMyNode.GetPort(), ret);
    }

    return ret;
}

TestSM * TestServer :: GetSM()
{
    return &m_oTestSM;
}
    
}


