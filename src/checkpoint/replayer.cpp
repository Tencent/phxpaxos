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

#include "replayer.h"
#include "phxpaxos/storage.h"
#include "sm_base.h"
#include "comm_include.h"
#include "config_include.h"
#include "cp_mgr.h"

namespace phxpaxos
{

Replayer :: Replayer(
    Config * poConfig, 
    SMFac * poSMFac, 
    LogStorage * poLogStorage, 
    CheckpointMgr * poCheckpointMgr)
    : m_poConfig(poConfig), 
    m_poSMFac(poSMFac), 
    m_oPaxosLog(poLogStorage), 
    m_poCheckpointMgr(poCheckpointMgr),
    m_bCanrun(false),
    m_bIsPaused(true),
    m_bIsEnd(false)
{
}

Replayer :: ~Replayer()
{
}

void Replayer :: Stop()
{
    m_bIsEnd = true;
    join();
}

void Replayer :: Pause()
{
    m_bCanrun = false;
}

void Replayer :: Continue()
{
    m_bIsPaused = false;
    m_bCanrun = true;
}

const bool Replayer:: IsPaused() const
{
    return m_bIsPaused;
}

void Replayer :: run()
{
    PLGHead("Checkpoint.Replayer [START]");
    uint64_t llInstanceID = m_poSMFac->GetCheckpointInstanceID(m_poConfig->GetMyGroupIdx()) + 1;

    while (true)
    {
        if (m_bIsEnd)
        {
            PLGHead("Checkpoint.Replayer [END]");
            return;
        }
        
        if (!m_bCanrun)
        {
            //PLGImp("Pausing, sleep");
            m_bIsPaused = true;
            Time::MsSleep(1000);
            continue;
        }
        
        if (llInstanceID >= m_poCheckpointMgr->GetMaxChosenInstanceID())
        {
            //PLGImp("now maxchosen instanceid %lu small than excute instanceid %lu, wait", 
                    //m_poCheckpointMgr->GetMaxChosenInstanceID(), llInstanceID);
            Time::MsSleep(1000);
            continue;
        }
        
        bool bPlayRet = PlayOne(llInstanceID);
        if (bPlayRet)
        {
            PLGImp("Play one done, instanceid %lu", llInstanceID);
            llInstanceID++;
        }
        else
        {
            PLGErr("Play one fail, instanceid %lu", llInstanceID);
            Time::MsSleep(500);
        }
    }
}

bool Replayer :: PlayOne(const uint64_t llInstanceID)
{
    AcceptorStateData oState; 
    int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), llInstanceID, oState);
    if (ret != 0)
    {
        return false;
    }

    bool bExecuteRet = m_poSMFac->ExecuteForCheckpoint(
            m_poConfig->GetMyGroupIdx(), llInstanceID, oState.acceptedvalue());
    if (!bExecuteRet)
    {
        PLGErr("Checkpoint sm excute fail, instanceid %lu", llInstanceID);
    }

    return bExecuteRet;
}

}


