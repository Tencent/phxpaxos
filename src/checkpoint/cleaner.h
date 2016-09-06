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

#include <typeinfo>
#include "utils_include.h"

namespace phxpaxos
{

#define CAN_DELETE_DELTA 1000000 
#define DELETE_SAVE_INTERVAL 100

class Config;
class SMFac;
class LogStorage;
class CheckpointMgr;

class Cleaner : public Thread
{
public:
    Cleaner(
            Config * poConfig,
            SMFac * poSMFac, 
            LogStorage * poLogStorage,
            CheckpointMgr * poCheckpointMgr);

    ~Cleaner();

    void Stop();

    void run();

    void Pause();

    void Continue();

    const bool IsPaused() const;

public:
    void SetHoldPaxosLogCount(const uint64_t llHoldCount);

    int FixMinChosenInstanceID(const uint64_t llOldMinChosenInstanceID);

private:
    bool DeleteOne(const uint64_t llInstanceID);

private:
    Config * m_poConfig;
    SMFac * m_poSMFac;
    LogStorage * m_poLogStorage;
    CheckpointMgr * m_poCheckpointMgr;

    uint64_t m_llLastSave;

    bool m_bCanrun;
    bool m_bIsPaused;

    bool m_bIsEnd;
    bool m_bIsStart;

    uint64_t m_llHoldCount;
};
    
}
