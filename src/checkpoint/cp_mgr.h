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

#include "replayer.h"
#include "cleaner.h"
#include "phxpaxos/options.h"
#include <set>

namespace phxpaxos
{

class CheckpointMgr
{
public:
    CheckpointMgr(
            Config * poConfig,
            SMFac * poSMFac, 
            LogStorage * poLogStorage,
            const bool bUseCheckpointReplayer);

    ~CheckpointMgr();

    int Init();

    void Start();

    void Stop();

    Replayer * GetReplayer();

    Cleaner * GetCleaner();

public:
    int PrepareForAskforCheckpoint(const nodeid_t iSendNodeID);

    const bool InAskforcheckpointMode() const;

    void ExitCheckpointMode();

public:
    const uint64_t GetMinChosenInstanceID() const;
    
    int SetMinChosenInstanceID(const uint64_t llMinChosenInstanceID);
    
    void SetMinChosenInstanceIDCache(const uint64_t llMinChosenInstanceID);

    const uint64_t GetCheckpointInstanceID() const;

    const uint64_t GetMaxChosenInstanceID() const;

    void SetMaxChosenInstanceID(const uint64_t llMaxChosenInstanceID);

private:
    Config * m_poConfig;
    LogStorage * m_poLogStorage;
    SMFac * m_poSMFac;
    
    Replayer m_oReplayer;
    Cleaner m_oCleaner;

    uint64_t m_llMinChosenInstanceID;
    uint64_t m_llMaxChosenInstanceID;

private:
    bool m_bInAskforCheckpointMode;
    std::set<nodeid_t> m_setNeedAsk;
    uint64_t m_llLastAskforCheckpointTime;

    bool m_bUseCheckpointReplayer;
};

}
