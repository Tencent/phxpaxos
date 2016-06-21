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

#include "utils_include.h"
#include "paxos_log.h"

namespace phxpaxos
{

class Config;
class SMFac;
class LogStorage;
class CheckpointMgr;
    
class Replayer : public Thread
{
public:
    Replayer(
            Config * poConfig,
            SMFac * poSMFac, 
            LogStorage * poLogStorage,
            CheckpointMgr * poCheckpointMgr);

    ~Replayer();

    void Stop();

    void run();

    void Pause();

    void Continue();

    const bool IsPaused() const;

private:
    bool PlayOne(const uint64_t llInstanceID);

private:
    Config * m_poConfig;
    SMFac * m_poSMFac;
    PaxosLog m_oPaxosLog;
    CheckpointMgr * m_poCheckpointMgr;

    bool m_bCanrun;
    bool m_bIsPaused;
    bool m_bIsEnd;
};

}
