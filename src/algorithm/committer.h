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
#include <inttypes.h>
#include "comm_include.h"
#include "sm_base.h"
#include "config_include.h"

namespace phxpaxos
{

class CommitCtx;
class IOLoop;

class Committer
{
public:
    Committer(Config * poConfig, CommitCtx * poCommitCtx, IOLoop * poIOLoop, SMFac * poSMFac);
    ~Committer();

public:
    int NewValueGetID(const std::string & sValue, uint64_t & llInstanceID);
    
    int NewValueGetID(const std::string & sValue, uint64_t & llInstanceID, SMCtx * poSMCtx);
    
    int NewValueGetIDNoRetry(const std::string & sValue, uint64_t & llInstanceID, SMCtx * poSMCtx);

    int NewValue(const std::string & sValue);

public:
    void SetTimeoutMs(const int iTimeoutMs);

    void SetMaxHoldThreads(const int iMaxHoldThreads);

    void SetProposeWaitTimeThresholdMS(const int iWaitTimeThresholdMS);

private:
    void LogStatus();

private:
    Config * m_poConfig;
    CommitCtx * m_poCommitCtx;
    IOLoop * m_poIOLoop;
    SMFac * m_poSMFac;

    WaitLock m_oWaitLock;
    int m_iTimeoutMs;

    uint64_t m_llLastLogTime;
};
    
}
