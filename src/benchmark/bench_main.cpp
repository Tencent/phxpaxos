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
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <sys/time.h>
#include "phxpaxos/options.h"
#include "utils_include.h"

using namespace bench;
using namespace phxpaxos;
using namespace std;

int parse_ipport(const char * pcStr, NodeInfo & oNodeInfo)
{
    char sIP[32] = {0};
    int iPort = -1;

    int count = sscanf(pcStr, "%[^':']:%d", sIP, &iPort);
    if (count != 2)
    {
        return -1;
    }

    oNodeInfo.SetIPPort(sIP, iPort);

    return 0;
}

int parse_ipport_list(const char * pcStr, NodeInfoList & vecNodeInfoList)
{
    string sTmpStr;
    int iStrLen = strlen(pcStr);

    for (int i = 0; i < iStrLen; i++)
    {
        if (pcStr[i] == ',' || i == iStrLen - 1)
        {
            if (i == iStrLen - 1 && pcStr[i] != ',')
            {
                sTmpStr += pcStr[i];
            }
            
            NodeInfo oNodeInfo;
            int ret = parse_ipport(sTmpStr.c_str(), oNodeInfo);
            if (ret != 0)
            {
                return ret;
            }

            vecNodeInfoList.push_back(oNodeInfo);

            sTmpStr = "";
        }
        else
        {
            sTmpStr += pcStr[i];
        }
    }

    return 0;
}

const uint64_t GetSteadyClockMS()
{
    uint64_t llNow;
    struct timeval tv; 

    gettimeofday(&tv, NULL);

    llNow = tv.tv_sec;
    llNow *= 1000;
    llNow += tv.tv_usec / 1000; 

    return llNow;
}

void RandValue(const int iSize, string & sValue)
{
    sValue = "";
    int iRealSize = rand() % iSize + iSize / 2;

    for (int i = 0; i < iRealSize; i++)
    {
        sValue += 'a';
    }
}

class BenchClient : public phxpaxos::Thread
{
public:
    BenchClient(BenchServer * poBenchServer, const int iWriteCount, const int iAvgValueSize) :
        m_poBenchServer(poBenchServer), m_iWriteCount(iWriteCount), m_iAvgValueSize(iAvgValueSize) { }

    ~BenchClient() { join(); }

    void run()
    {
        string sValue;
        for (int i = 0; i < m_iWriteCount; i++)
        {
            RandValue(m_iAvgValueSize, sValue);
            int ret = m_poBenchServer->Write(sValue);
            if (ret != 0)
            {
                printf("bench fail\n");
                return;
            }
        }
    }

private:
    BenchServer * m_poBenchServer;
    int m_iWriteCount;
    int m_iAvgValueSize;
};

void Bench_SmallData(BenchServer & oBenchServer)
{
    uint64_t llBeginTimeMs = GetSteadyClockMS();

    const int iWriteCount = 1000;
    const int iConcurrentCount = 100;
    int iAvgValueSize = 100;

    vector<BenchClient *> vecBenchClient;
    for (int i = 0; i < iConcurrentCount; i++)
    {
        BenchClient * poBenchClient = new BenchClient(&oBenchServer, iWriteCount, iAvgValueSize);
        poBenchClient->start();
        vecBenchClient.push_back(poBenchClient);
    }

    for (int i = 0; i < iConcurrentCount; i++)
    {
        delete vecBenchClient[i];
    }

    uint64_t llEndTimeMs = GetSteadyClockMS();
    int iRunTimeMs = llEndTimeMs - llBeginTimeMs;
    int qps = (uint64_t)iWriteCount * iConcurrentCount * 1000 / iRunTimeMs;

    printf("avg value size %d qps %d\n", iAvgValueSize, qps);
}

void Bench_LargeData(BenchServer & oBenchServer)
{
    uint64_t llBeginTimeMs = GetSteadyClockMS();

    const int iWriteCount = 100;
    const int iConcurrentCount = 100;
    int iAvgValueSize = 100 * 1000;

    vector<BenchClient *> vecBenchClient;
    for (int i = 0; i < iConcurrentCount; i++)
    {
        BenchClient * poBenchClient = new BenchClient(&oBenchServer, iWriteCount, iAvgValueSize);
        poBenchClient->start();
        vecBenchClient.push_back(poBenchClient);
    }

    for (int i = 0; i < iConcurrentCount; i++)
    {
        delete vecBenchClient[i];
    }

    uint64_t llEndTimeMs = GetSteadyClockMS();
    int iRunTimeMs = llEndTimeMs - llBeginTimeMs;
    int qps = (uint64_t)iWriteCount * iConcurrentCount * 1000 / iRunTimeMs;

    printf("avg value size %d qps %d\n", iAvgValueSize, qps);
}

void Bench(BenchServer & oBenchServer)
{
    int ret = oBenchServer.ReadyBench();
    if (ret != 0)
    {
        printf("bench start fail\n");
        return;
    }

    printf("start bench small data\n");
    Bench_SmallData(oBenchServer);
    printf("start bench large data\n");
    Bench_LargeData(oBenchServer);
}

int main(int argc, char ** argv)
{
    if (argc < 4)
    {
        printf("%s <myip:myport> <node0_ip:node_0port,node1_ip:node_1_port,node2_ip:node2_port,...> "
                "<start bench? y/n> <paxos group count>\n", argv[0]);
        return -1;
    }

    NodeInfo oMyNode;
    if (parse_ipport(argv[1], oMyNode) != 0)
    {
        printf("parse myip:myport fail\n");
        return -1;
    }

    NodeInfoList vecNodeInfoList;
    if (parse_ipport_list(argv[2], vecNodeInfoList) != 0)
    {
        printf("parse ip/port list fail\n");
        return -1;
    }

    int iGroupCount = 1;
    if (argc >= 5)
    {
        iGroupCount = atoi(argv[4]);
    }

    BenchServer oBenchServer(iGroupCount, oMyNode, vecNodeInfoList);
    int ret = oBenchServer.RunPaxos();
    if (ret != 0)
    {
        printf("run paxos fail\n");
        return -1;
    }

    string sStartBench = argv[3];
    if (sStartBench == "y")
    {
        Bench(oBenchServer);
    }
    else
    {
        //run damon
        while(true) sleep(1);
    }

    return 0;
}


