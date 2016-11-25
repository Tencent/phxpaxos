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
#include "phxpaxos/options.h"
#include "phxpaxos/sm.h"

namespace phxpaxos
{

class Learner;
class Config;
class SMFac;
class CheckpointMgr;

#define Checkpoint_ACK_TIMEOUT 120000
#define Checkpoint_ACK_LEAD 10 

class CheckpointSender : public Thread
{
public:
    CheckpointSender(
            const nodeid_t iSendNodeID,
            Config * poConfig, 
            Learner * poLearner,
            SMFac * poSMFac, 
            CheckpointMgr * poCheckpointMgr);

    ~CheckpointSender();

    void Stop();

    void run();

    void End();

    const bool IsEnd() const;

    void Ack(const nodeid_t iSendNodeID, const uint64_t llUUID, const uint64_t llSequence);

private:
    void SendCheckpoint();

    int LockCheckpoint();

    void UnLockCheckpoint();

    int SendCheckpointFofaSM(StateMachine * poSM);

    int SendFile(const StateMachine * poSM, const std::string & sDirPath, const std::string & sFilePath);

    int SendBuffer(const int iSMID, const uint64_t llCheckpointInstanceID, const std::string & sFilePath,
            const uint64_t llOffset, const std::string & sBuffer);

    const bool CheckAck(const uint64_t llSendSequence);

private:
    nodeid_t m_iSendNodeID;

    Config * m_poConfig;
    Learner * m_poLearner;
    SMFac * m_poSMFac;
    CheckpointMgr * m_poCheckpointMgr;

    bool m_bIsEnd;
    bool m_bIsEnded;
    bool m_bIsStarted;

private:
    uint64_t m_llUUID;
    uint64_t m_llSequence;

private:
    uint64_t m_llAckSequence;
    uint64_t m_llAbsLastAckTime;

private:
    char m_sTmpBuffer[1048576];
    
    std::map<std::string, bool> m_mapAlreadySendedFile;
};
    
}
