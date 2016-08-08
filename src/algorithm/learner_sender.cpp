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

#include "learner_sender.h"
#include "learner.h"

namespace phxpaxos
{

LearnerSender :: LearnerSender(Config * poConfig, Learner * poLearner, PaxosLog * poPaxosLog)
    : m_poConfig(poConfig), m_poLearner(poLearner), m_poPaxosLog(poPaxosLog)
{
    m_bIsEnd = false;
    m_bIsStart = false;
    SendDone();
}

LearnerSender :: ~LearnerSender()
{
}

void LearnerSender :: Stop()
{
    m_bIsEnd = true;
    if (m_bIsStart)
    {
        join();
    }
}

void LearnerSender :: run()
{
    m_bIsStart = true;

    while (true)
    {
        WaitToSend();

        if (m_bIsEnd)
        {
            PLGHead("Learner.Sender [END]");
            return;
        }

        SendLearnedValue(m_llBeginInstanceID, m_iSendToNodeID);

        SendDone();
    }
}

////////////////////////////////////////

void LearnerSender :: ReleshSending()
{
    m_llAbsLastSendTime = Time::GetSteadyClockMS();
}

const bool LearnerSender :: IsIMSending()
{
    if (!m_bIsIMSending)
    {
        return false;
    }

    uint64_t llNowTime = Time::GetSteadyClockMS();
    uint64_t llPassTime = llNowTime > m_llAbsLastSendTime ? llNowTime - m_llAbsLastSendTime : 0;

    if ((int)llPassTime >= LearnerSender_PREPARE_TIMEOUT)
    {
        return false;
    }

    return true;
}

const bool LearnerSender :: CheckAck(const uint64_t llSendInstanceID)
{
    while (llSendInstanceID > m_llAckInstanceID + LearnerSender_ACK_LEAD)
    {
        uint64_t llNowTime = Time::GetSteadyClockMS();
        uint64_t llPassTime = llNowTime > m_llAbsLastAckTime ? llNowTime - m_llAbsLastAckTime : 0;

        if ((int)llPassTime >= LearnerSender_ACK_TIMEOUT)
        {
            BP->GetLearnerBP()->SenderAckTimeout();
            PLGErr("Ack timeout, last acktime %lu", m_llAbsLastAckTime);
            return false;
        }

        BP->GetLearnerBP()->SenderAckDelay();
        //PLGErr("Need sleep to slow down send speed, sendinstaceid %lu ackinstanceid %lu",
                //llSendInstanceID, m_llAckInstanceID);
        Time::MsSleep(50);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

const bool LearnerSender :: Prepare(const uint64_t llBeginInstanceID, const nodeid_t iSendToNodeID)
{
    m_oLock.Lock();
    
    bool bPrepareRet = false;
    if (!IsIMSending() && !m_bIsComfirmed)
    {
        bPrepareRet = true;

        m_bIsIMSending = true;
        m_llAbsLastSendTime = m_llAbsLastAckTime = Time::GetSteadyClockMS();
        m_llBeginInstanceID = m_llAckInstanceID = llBeginInstanceID;
        m_iSendToNodeID = iSendToNodeID;
    }
    
    m_oLock.UnLock();

    return bPrepareRet;
}

const bool LearnerSender :: Comfirm(const uint64_t llBeginInstanceID, const nodeid_t iSendToNodeID)
{
    m_oLock.Lock();

    bool bComfirmRet = false;

    if (IsIMSending() && (!m_bIsComfirmed))
    {
        if (m_llBeginInstanceID == llBeginInstanceID && m_iSendToNodeID == iSendToNodeID)
        {
            bComfirmRet = true;

            m_bIsComfirmed = true;
            m_oLock.Interupt();
        }
    }

    m_oLock.UnLock();

    return bComfirmRet;
}

void LearnerSender :: Ack(const uint64_t llAckInstanceID, const nodeid_t iFromNodeID)
{
    m_oLock.Lock();

    if (IsIMSending() && m_bIsComfirmed)
    {
        if (m_iSendToNodeID == iFromNodeID)
        {
            if (llAckInstanceID > m_llAckInstanceID)
            {
                m_llAckInstanceID = llAckInstanceID;
                m_llAbsLastAckTime = Time::GetSteadyClockMS();
            }
        }
    }

    m_oLock.UnLock();
}    

///////////////////////////////////////////////

void LearnerSender :: WaitToSend()
{
    m_oLock.Lock();
    while (!m_bIsComfirmed)
    {
        m_oLock.WaitTime(1000);
        if (m_bIsEnd)
        {
            break;
        }
    }
    m_oLock.UnLock();
}

void LearnerSender :: SendLearnedValue(const uint64_t llBeginInstanceID, const nodeid_t iSendToNodeID)
{
    PLGHead("BeginInstanceID %lu SendToNodeID %lu", llBeginInstanceID, iSendToNodeID);

    uint64_t llSendInstanceID = llBeginInstanceID;
    int ret = 0;
    
    uint32_t iLastChecksum = 0;

    while (llSendInstanceID < m_poLearner->GetInstanceID())
    {    
        ret = SendOne(llSendInstanceID, iSendToNodeID, iLastChecksum);
        if (ret != 0)
        {
            PLGErr("SendOne fail, SendInstanceID %lu SendToNodeID %lu ret %d",
                    llSendInstanceID, iSendToNodeID, ret);
            return;
        }

        if (!CheckAck(llSendInstanceID))
        {
            return;
        }

        llSendInstanceID++;
        ReleshSending();
    }
}

int LearnerSender :: SendOne(const uint64_t llSendInstanceID, const nodeid_t iSendToNodeID, uint32_t & iLastChecksum)
{
    BP->GetLearnerBP()->SenderSendOnePaxosLog();

    AcceptorStateData oState;
    int ret = m_poPaxosLog->ReadState(m_poConfig->GetMyGroupIdx(), llSendInstanceID, oState);
    if (ret != 0)
    {
        return ret;
    }

    BallotNumber oBallot(oState.acceptedid(), oState.acceptednodeid());

    ret = m_poLearner->SendLearnValue(iSendToNodeID, llSendInstanceID, oBallot, oState.acceptedvalue(), iLastChecksum);

    iLastChecksum = oState.checksum();

    return ret;
}

void LearnerSender :: SendDone()
{
    m_oLock.Lock();

    m_bIsIMSending = false;
    m_bIsComfirmed = false;
    m_llBeginInstanceID = (uint64_t)-1;
    m_iSendToNodeID = nullnode;
    m_llAbsLastSendTime = 0;
    
    m_llAckInstanceID = 0;
    m_llAbsLastAckTime = 0;

    m_oLock.UnLock();
}

    
}


