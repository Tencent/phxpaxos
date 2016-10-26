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

#include <map>
#include "timer.h"
#include "utils_include.h"
#include <string>
#include "comm_include.h"
#include <queue>
#include "config_include.h"

namespace phxpaxos
{

#define RETRY_QUEUE_MAX_LEN 300

class Instance;

class IOLoop : public Thread
{
public:
    IOLoop(Config * poConfig, Instance * poInstance);
    virtual ~IOLoop();

    void run();

    void Stop();

    void OneLoop(const int iTimeoutMs);

    void DealWithRetry();

    void ClearRetryQueue();

public:
    int AddMessage(const char * pcMessage, const int iMessageLen);

    int AddRetryPaxosMsg(const PaxosMsg & oPaxosMsg);

    void AddNotify();

public:
    virtual bool AddTimer(const int iTimeout, const int iType, uint32_t & iTimerID);

    virtual void RemoveTimer(uint32_t & iTimerID);

    void DealwithTimeout(int & iNextTimeout);

    void DealwithTimeoutOne(const uint32_t iTimerID, const int iType);

private:
    bool m_bIsEnd;
    bool m_bIsStart;
    Timer m_oTimer;
    std::map<uint32_t, bool> m_mapTimerIDExist;

    Queue<std::string *> m_oMessageQueue;
    std::queue<PaxosMsg> m_oRetryQueue;

    int m_iQueueMemSize;

    Config * m_poConfig;
    Instance * m_poInstance;
};
    
}
