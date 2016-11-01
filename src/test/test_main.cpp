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
#include <mutex>
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
#include <map>
#include <vector>
#include <algorithm>

using namespace phxpaxos_test;
using namespace phxpaxos;
using namespace std;

int giRunNodeCount = 3;
bool gbTestBatch = false;

void RandValue(const int iSize, string & sValue)
{
    sValue = "";
    int iRealSize = rand() % iSize + iSize / 2;

    for (int i = 0; i < iRealSize; i++)
    {
        sValue += 'a';
    }
}

class TestSuccWrite
{
public:
    TestSuccWrite() 
    { 
        m_llMaxInstanceID = 0;
    }

    int AddSuccWrite(const uint64_t llInstanceID, const uint32_t iBatchIndex, const string & sWriteValue)
    {
        m_oMutex.lock();
        if (llInstanceID < m_llMaxInstanceID)
        {
            m_oMutex.unlock();
            return -1;
        }
        m_llMaxInstanceID = llInstanceID;

        SuccWriteValue oValue;
        oValue.sValue = sWriteValue;
        oValue.llInstanceID = llInstanceID;
        oValue.iBatchIndex = iBatchIndex;
        m_vecSuccWrite.push_back(oValue);
        m_oMutex.unlock();

        return 0;
    }

    vector<pair<uint64_t, string> > ToVector()
    {
        sort(m_vecSuccWrite.begin(), m_vecSuccWrite.end());
        vector<pair<uint64_t, string> > vecSuccWrite;
        for (size_t i = 0; i < m_vecSuccWrite.size(); i++)
        {
            vecSuccWrite.push_back(make_pair(m_vecSuccWrite[i].llInstanceID, m_vecSuccWrite[i].sValue));
        }
        return vecSuccWrite;
    }

    struct SuccWriteValue
    {
        string sValue;
        uint64_t llInstanceID;
        uint32_t iBatchIndex;

        bool operator < (const SuccWriteValue & obj) const
        {
            if (llInstanceID == obj.llInstanceID)
            {
                return iBatchIndex < obj.iBatchIndex;
            }
            else
            {
                return llInstanceID < obj.llInstanceID;
            }
        }
    };

public:
    std::mutex m_oMutex;
    vector<SuccWriteValue> m_vecSuccWrite;
    uint64_t m_llMaxInstanceID;
};

TestSuccWrite goSuccWrite;

class TestClient : public phxpaxos::Thread
{
public:
    TestClient(TestServer * poTestServer, const int iWriteCount, const int iAvgValueSize) :
        m_poTestServer(poTestServer), m_iWriteCount(iWriteCount), m_iAvgValueSize(iAvgValueSize), m_iRunRet(0) { }

    ~TestClient() { }

    void run()
    {
        string sValue;
        m_iAvgValueSize = 100 * 1024;
        for (int i = 0; i < m_iWriteCount; i++)
        {
            RandValue(m_iAvgValueSize, sValue);

            while (true)
            {
                uint64_t llInstanceID = 0;
                uint32_t iBatchIndex = 0;
                int ret = -1;
                if (gbTestBatch)
                {
                    ret = m_poTestServer->BatchWrite(sValue, llInstanceID, iBatchIndex);
                }
                else
                {
                    ret = m_poTestServer->Write(sValue, llInstanceID);
                }

                if (ret != PaxosTryCommitRet_OK && ret != PaxosTryCommitRet_Conflict)
                {
                    printf("write fail, ret %d\n", ret);
                    m_iRunRet = -1;
                    return;
                }
                else if (ret == PaxosTryCommitRet_Conflict)
                {
                    continue;
                }
                else
                {
                    ret = goSuccWrite.AddSuccWrite(llInstanceID, iBatchIndex, sValue);
                    if (ret != 0)
                    {
                        printf("this instance already exist, is error, instanceid %lu valuesize %zu ret %d\n", 
                                llInstanceID, sValue.size(), ret);
                        m_iRunRet = -1;
                        return;
                    }
                    break;
                }
            }
        }
    }

    const int RunRet() { return m_iRunRet; }

private:
    TestServer * m_poTestServer;
    int m_iWriteCount;
    int m_iAvgValueSize;

    int m_iRunRet;
};

class TestClientPool
{
public:
    TestClientPool(TestServer * poTestServer, const int iWriteCount, const int iAvgValueSize)
    {
        int iClientCount = 100;
        for (int i = 0; i < iClientCount; i++)
        {
            auto poClient = new TestClient(poTestServer, iWriteCount / iClientCount, iAvgValueSize);
            poClient->start();
            m_vecTestClient.push_back(poClient);
        }
    }

    ~TestClientPool()
    {
        for (auto & poClient : m_vecTestClient)
        {
            delete poClient;
        }
    }

    void Stop()
    {
        for (auto & poClient : m_vecTestClient)
        {
            poClient->join();
        }
    }

    const int RunRet()
    {
        for (auto & poClient : m_vecTestClient)
        {
            if (poClient->RunRet() != 0)
            {
                return poClient->RunRet();
            }
        }

        return 0;
    }

private:
    vector<TestClient *> m_vecTestClient;
};

/////////////////////////////////////////////////////////////////////////////
bool CheckCorrect(vector<TestServer *> & vecTestServerList)
{
    for (auto & poTestServer : vecTestServerList)
    {
        if (!poTestServer->GetSM()->CheckExecuteValueCorrect(goSuccWrite.ToVector()))
        {
            return false;
        }
    }

    return true;
}

int RunServer(vector<TestServer *> & vecTestServerList)
{
    string sIP = "127.0.0.1";
    int iPort = 11112;
    int iServerCount = giRunNodeCount;
    NodeInfoList vecNodeList;

    for (int i = 0; i < iServerCount; i++)
    {
        NodeInfo oNode(sIP, iPort + i);
        vecNodeList.push_back(oNode);
    }

    for (int i = 0; i < iServerCount; i++)
    {
        NodeInfo oMyNode(sIP, iPort + i);
        TestServer * poTestServer = new TestServer(oMyNode, vecNodeList);
        assert(poTestServer != nullptr);
        vecTestServerList.push_back(poTestServer);

        int ret = poTestServer->RunPaxos();
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

int ReadyServer(vector<TestServer *> & vecTestServerList)
{
    //wait all thread is started.
    Time::MsSleep(50);

    for (auto & poTestServer : vecTestServerList)
    {
        int ret = poTestServer->Ready();
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

void EndServer(vector<TestServer *> & vecTestServerList)
{
    for (auto & poTestServer : vecTestServerList)
    {
        delete poTestServer;
    }
}

void ShutDownOneNode(vector<TestServer *> & vecTestServerList)
{
    printf("\nstart shutdown one node........\n");
    if (vecTestServerList.size() > 0)
    {
        delete vecTestServerList[0];
        vecTestServerList.erase(vecTestServerList.begin());
    }
}

/////////////////////////////////////////////////////////////

bool Test_OnlyOneNode_Write(vector<TestServer *> & vecTestServerList)
{
    TimeStat oTimeStat;
    oTimeStat.Point();

    TestServer * poTestServer = vecTestServerList[0];

    TestClientPool oClient(poTestServer, 100, 20);
    oClient.Stop();

    printf("write done, usetime %dms, runret %s, nowsuccwritecount %zu\n",
            oTimeStat.Point(), oClient.RunRet() == 0 ? "succ" : "fail", goSuccWrite.m_vecSuccWrite.size());

    if (oClient.RunRet() != 0)
    {
        return false;
    }

    printf("wait 5 second let all node learn to latest...\n");
    Time::MsSleep(5000);

    if (!CheckCorrect(vecTestServerList))
    {
        return false;
    }

    return true;
}

bool Test_AllNode_Write_Parallel(vector<TestServer *> & vecTestServerList)
{
    TimeStat oTimeStat;
    oTimeStat.Point();

    vector<TestClientPool *> vecClient;
    for (auto & poTestServer : vecTestServerList)
    {
        TestClientPool * poClient = new TestClientPool(poTestServer, 100, 20);
        vecClient.push_back(poClient);
    }

    bool bRunRet = true;
    for (auto & poClient : vecClient)
    {
        poClient->Stop();
        if (poClient->RunRet() != 0)
        {
            bRunRet = false;
        }
        delete poClient;
    }

    printf("write done, usetime %dms, runret %s, succwritecount %zu\n",
            oTimeStat.Point(), bRunRet ? "succ" : "fail", goSuccWrite.m_vecSuccWrite.size());
    if (!bRunRet)
    {
        return false;
    }

    printf("wait 5 second let all node learn to latest...\n");
    Time::MsSleep(5000);

    if (!CheckCorrect(vecTestServerList))
    {
        return false;
    }

    return true;
}

void Test(vector<TestServer *> & vecTestServerList)
{
    printf("\nstart test round, runnode %zu\n", vecTestServerList.size());
    printf("start Test_OnlyOneNode_Write................\n");
    if (Test_OnlyOneNode_Write(vecTestServerList))
    {
        printf("Test_OnlyOneNode_Write pass...\n");
    }
    else
    {
        printf("Test_OnlyOneNode_Write fail...\n");
    }

    printf("\nstart Test_AllNode_Write_Parallel................\n");
    if (Test_AllNode_Write_Parallel(vecTestServerList))
    {
        printf("Test_AllNode_Write_Parallel pass...\n");
    }
    else
    {
        printf("Test_AllNode_Write_Parallel fail...\n");
    }
}

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("%s <run node count> <test batch y/n ?>\n", argv[0]);
        return -1;
    }

    giRunNodeCount = atoi(argv[1]);
    if (argc >= 3)
    {
        gbTestBatch = string(argv[2]) == "y" ? true : false;
    }

    vector<TestServer *> vecTestServerList;

    int ret = RunServer(vecTestServerList);
    if (ret != 0)
    {
        EndServer(vecTestServerList);
        return -1;
    }

    ret = ReadyServer(vecTestServerList);
    if (ret != 0)
    {
        EndServer(vecTestServerList);
        return -1;
    }

    Test(vecTestServerList);

    while ((int)vecTestServerList.size() > (giRunNodeCount / 2 + 1))
    {
        ShutDownOneNode(vecTestServerList);
        Test(vecTestServerList);
    }

    EndServer(vecTestServerList);

    return 0;
}


