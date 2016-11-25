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

#pragma once

#include "commdef.h"
#include "utils_include.h"
#include "phxpaxos/node.h"
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>

namespace phxpaxos
{

class PendingProposal
{
public:
    PendingProposal();
    const std::string * psValue;
    SMCtx * poSMCtx;

    //return parameter
    uint64_t * pllInstanceID; 
    uint32_t * piBatchIndex;

    //notify
    Notifier * poNotifier;

    uint64_t llAbsEnqueueTime;
};

///////////////////////////////////

class ProposeBatch
{
public:
    ProposeBatch(const int iGroupIdx, Node * poPaxosNode, NotifierPool * poNotifierPool);
    virtual ~ProposeBatch();

    void Start();

    void Run();

    void Stop();

    int Propose(const std::string & sValue, uint64_t & llInstanceID, uint32_t & iBatchIndex, SMCtx * poSMCtx);

public:
    void SetBatchCount(const int iBatchCount);
    void SetBatchDelayTimeMs(const int iBatchDelayTimeMs);

protected:
    virtual void DoPropose(std::vector<PendingProposal> & vecRequest);

private:
    void AddProposal(const std::string & sValue, uint64_t & llInstanceID, uint32_t & iBatchIndex, 
            SMCtx * poSMCtx, Notifier * poNotifier);
    void PluckProposal(std::vector<PendingProposal> & vecRequest);
    void OnlyOnePropose(PendingProposal & oPendingProposal);
    const bool NeedBatch();

private:
    const int m_iMyGroupIdx;
    Node * m_poPaxosNode;
    NotifierPool * m_poNotifierPool;

    std::mutex m_oMutex;
    std::condition_variable m_oCond;
    std::queue<PendingProposal> m_oQueue;
    bool m_bIsEnd;
    bool m_bIsStarted;
    int m_iNowQueueValueSize;

private:
    int m_iBatchCount;
    int m_iBatchDelayTimeMs;
    int m_iBatchMaxSize;

    std::thread * m_poThread;
};

}
