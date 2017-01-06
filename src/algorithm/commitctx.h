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

#include <string>
#include "comm_include.h"
#include "config_include.h"

namespace phxpaxos
{

class StateMachine;

class CommitCtx
{
public:
    CommitCtx(Config * poConfig);
    ~CommitCtx();

    void NewCommit(std::string * psValue, SMCtx * poSMCtx, const int iTimeoutMs);
    
    const bool IsNewCommit() const;

    std::string & GetCommitValue();

    void StartCommit(const uint64_t llInstanceID);

    bool IsMyCommit(const uint64_t llInstanceID, const std::string & sLearnValue, SMCtx *& poSMCtx);

public:
    void SetResult(const int iCommitRet, const uint64_t llInstanceID, const std::string & sLearnValue);

    void SetResultOnlyRet(const int iCommitRet);

    int GetResult(uint64_t & llSuccInstanceID);

public:
    const int GetTimeoutMs() const;

private:
    Config * m_poConfig;

    uint64_t m_llInstanceID;
    int m_iCommitRet;
    bool m_bIsCommitEnd;
    int m_iTimeoutMs;

    std::string * m_psValue;
    SMCtx * m_poSMCtx;
    SerialLock m_oSerialLock;
};
}
