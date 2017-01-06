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

#include "commitctx.h"
#include "phxpaxos/sm.h"

namespace phxpaxos
{

CommitCtx :: CommitCtx(Config * poConfig)
    : m_poConfig(poConfig)
{
    NewCommit(nullptr, nullptr, 0);
}

CommitCtx :: ~CommitCtx()
{
}

void CommitCtx :: NewCommit(std::string * psValue, SMCtx * poSMCtx, const int iTimeoutMs)
{
    m_oSerialLock.Lock();

    m_llInstanceID = (uint64_t)-1;
    m_iCommitRet = -1;
    m_bIsCommitEnd = false;
    m_iTimeoutMs = iTimeoutMs;

    m_psValue = psValue;
    m_poSMCtx = poSMCtx;

    if (psValue != nullptr)
    {
        PLGHead("OK, valuesize %zu", psValue->size());
    }

    m_oSerialLock.UnLock();
}


const bool CommitCtx :: IsNewCommit() const
{
    return m_llInstanceID == (uint64_t)-1 && m_psValue != nullptr;
}

std::string & CommitCtx :: GetCommitValue()
{
    return *m_psValue;
}

void CommitCtx :: StartCommit(const uint64_t llInstanceID)
{
    m_oSerialLock.Lock();
    m_llInstanceID = llInstanceID;
    m_oSerialLock.UnLock();
}

bool CommitCtx :: IsMyCommit(const uint64_t llInstanceID, const std::string & sLearnValue,  SMCtx *& poSMCtx)
{
    m_oSerialLock.Lock();

    bool bIsMyCommit = false;

    if ((!m_bIsCommitEnd) && (m_llInstanceID == llInstanceID))
    {
        bIsMyCommit = (sLearnValue == (*m_psValue));
    }

    if (bIsMyCommit)
    {
        poSMCtx = m_poSMCtx;
    }

    m_oSerialLock.UnLock();

    return bIsMyCommit;
}

void CommitCtx :: SetResultOnlyRet(const int iCommitRet)
{
    return SetResult(iCommitRet, (uint64_t)-1, "");
}

void CommitCtx :: SetResult(
        const int iCommitRet, 
        const uint64_t llInstanceID, 
        const std::string & sLearnValue)
{
    m_oSerialLock.Lock();

    if (m_bIsCommitEnd || (m_llInstanceID != llInstanceID))
    {
        m_oSerialLock.UnLock();
        return;
    }

    m_iCommitRet = iCommitRet;

    if (m_iCommitRet == 0)
    {
        if ((*m_psValue) != sLearnValue)
        {
            m_iCommitRet = PaxosTryCommitRet_Conflict;
        }
    }

    m_bIsCommitEnd = true;
    m_psValue = nullptr;

    m_oSerialLock.Interupt();
    m_oSerialLock.UnLock();
}


int CommitCtx :: GetResult(uint64_t & llSuccInstanceID)
{
    m_oSerialLock.Lock();

    while (!m_bIsCommitEnd)
    {
        m_oSerialLock.WaitTime(1000);
    }

    if (m_iCommitRet == 0)
    {
        llSuccInstanceID = m_llInstanceID;
        PLGImp("commit success, instanceid %lu", llSuccInstanceID);
    }
    else
    {
        PLGErr("commit fail, ret %d", m_iCommitRet);
    }
    
    m_oSerialLock.UnLock();

    return m_iCommitRet;
}

const int CommitCtx :: GetTimeoutMs() const
{
    return m_iTimeoutMs;
}

}


