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

#include "propose_batch.h"
#include <algorithm>
#include "utils_include.h"

using namespace phxpaxos;
using namespace std;

class RequestClient : public Thread
{
public:
    RequestClient(ProposeBatch * poBatch)
        : m_poBatch(poBatch) { }
    ~RequestClient() { join(); }

    void run()
    {
        while (true)
        {
            //Time::MsSleep(rand() % 10000);
            uint64_t llInstanceID = 0;
            uint32_t iBatchIndex = 0;
            string sValue = "hello paxos";
            int ret = m_poBatch->Propose(sValue, llInstanceID, iBatchIndex, nullptr);
            
            printf("propose done, instanceid %lu batchindex %u ret %d\n", 
                    llInstanceID, iBatchIndex, ret);
        }
    }

private:
    ProposeBatch * m_poBatch;
};

class ProposeBatchTest : public ProposeBatch
{
public:
    ProposeBatchTest(const int iGroupIdx, Node * poPaxosNode, NotifierPool * poNotifierPool)
        : ProposeBatch(iGroupIdx, poPaxosNode, poNotifierPool) { }

    void DoPropose(std::vector<PendingProposal> & vecRequest)
    {
        if (vecRequest.size() == 0)
        {
            return;
        }

        Time::MsSleep(rand() % 1000);
        for (size_t i = 0; i < vecRequest.size(); i++)
        {
            PendingProposal & oPendingProposal = vecRequest[i];
            *oPendingProposal.piBatchIndex = (uint32_t)i;
            *oPendingProposal.pllInstanceID = 1024; 
            oPendingProposal.poNotifier->SendNotify(0);
        }
        printf("batch size %zu\n", vecRequest.size());
    }
};

int main(int argc, char ** argv)
{
    NotifierPool oNotifierPool;
    ProposeBatchTest oBatch(0, nullptr, &oNotifierPool);
    oBatch.SetBatchDelayTimeMs(1000);

    int iClientCount = 10;
    vector<RequestClient *> vecClient;
    for (int i = 0; i < iClientCount; i++)
    {
        auto poClient = new RequestClient(&oBatch);
        poClient->start();
        vecClient.push_back(poClient);
    }

    for (int i = 0; i < iClientCount; i++)
    {
        delete vecClient[i];
    }

    return 0;
}

