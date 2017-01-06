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
#include "comm_include.h"
#include "config_include.h"
#include "paxos_log.h"

namespace phxpaxos
{

class Learner;

class LearnerSender : public Thread
{
public:
    LearnerSender(Config * poConfig, Learner * poLearner, PaxosLog * poPaxosLog);
    ~LearnerSender();

    void run();

    void Stop();

public:
    const bool Prepare(const uint64_t llBeginInstanceID, const nodeid_t iSendToNodeID);

    const bool Comfirm(const uint64_t llBeginInstanceID, const nodeid_t iSendToNodeID);

    void Ack(const uint64_t llAckInstanceID, const nodeid_t iFromNodeID);

private:
    void WaitToSend();

    void SendLearnedValue(const uint64_t llBeginInstanceID, const nodeid_t iSendToNodeID);

    int SendOne(const uint64_t llSendInstanceID, const nodeid_t iSendToNodeID, uint32_t & iLastChecksum);

    void SendDone();

    const bool IsIMSending();

    void ReleshSending();

    const bool CheckAck(const uint64_t llSendInstanceID);

    void CutAckLead();

private:
    Config * m_poConfig;
    Learner * m_poLearner;
    PaxosLog * m_poPaxosLog;
    SerialLock m_oLock;

    bool m_bIsIMSending;
    uint64_t m_llAbsLastSendTime;

    uint64_t m_llBeginInstanceID;
    nodeid_t m_iSendToNodeID;

    bool m_bIsComfirmed;

    uint64_t m_llAckInstanceID;
    uint64_t m_llAbsLastAckTime;
    int m_iAckLead;

    bool m_bIsEnd;
    bool m_bIsStart;
};
    
}
