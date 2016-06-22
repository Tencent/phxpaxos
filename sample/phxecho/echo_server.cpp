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

#include "echo_server.h"
#include <assert.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace phxpaxos;
using namespace std;

namespace phxecho
{

PhxEchoServer :: PhxEchoServer(const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList)
    : m_oMyNode(oMyNode), m_vecNodeList(vecNodeList), m_poPaxosNode(nullptr)
{
}

PhxEchoServer :: ~PhxEchoServer()
{
    delete m_poPaxosNode;
}

int PhxEchoServer :: MakeLogStoragePath(std::string & sLogStoragePath)
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

int PhxEchoServer :: RunPaxos()
{
    Options oOptions;

    int ret = MakeLogStoragePath(oOptions.sLogStoragePath);
    if (ret != 0)
    {
        return ret;
    }

    //this groupcount means run paxos group count.
    //every paxos group is independent, there are no any communicate between any 2 paxos group.
    oOptions.iGroupCount = 1;

    oOptions.oMyNode = m_oMyNode;
    oOptions.vecNodeInfoList = m_vecNodeList;

    GroupSMInfo oSMInfo;
    oSMInfo.iGroupIdx = 0;
    //one paxos group can have multi state machine.
    oSMInfo.vecSMList.push_back(&m_oEchoSM);
    oOptions.vecGroupSMInfoList.push_back(oSMInfo);

    //use logger_google to print log
    LogFunc pLogFunc;
    ret = LoggerGoogle :: GetLogger("phxecho", "./log", 3, pLogFunc);
    if (ret != 0)
    {
        printf("get logger_google fail, ret %d\n", ret);
        return ret;
    }

    //set logger
    oOptions.pLogFunc = pLogFunc;

    ret = Node::RunNode(oOptions, m_poPaxosNode);
    if (ret != 0)
    {
        printf("run paxos fail, ret %d\n", ret);
        return ret;
    }

    printf("run paxos ok\n");
    return 0;
}

int PhxEchoServer :: Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue)
{
    SMCtx oCtx;
    PhxEchoSMCtx oEchoSMCtx;
    //smid must same to PhxEchoSM.SMID().
    oCtx.m_iSMID = 1;
    oCtx.m_pCtx = (void *)&oEchoSMCtx;

    uint64_t llInstanceID = 0;
    int ret = m_poPaxosNode->Propose(0, sEchoReqValue, llInstanceID, &oCtx);
    if (ret != 0)
    {
        printf("paxos propose fail, ret %d\n", ret);
        return ret;
    }

    if (oEchoSMCtx.iExecuteRet != 0)
    {
        printf("echo sm excute fail, excuteret %d\n", oEchoSMCtx.iExecuteRet);
        return oEchoSMCtx.iExecuteRet;
    }

    sEchoRespValue = oEchoSMCtx.sEchoRespValue.c_str();

    return 0;
}
    
}

