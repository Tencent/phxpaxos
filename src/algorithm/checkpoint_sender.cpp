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

#include "checkpoint_sender.h"
#include "comm_include.h"
#include "learner.h"
#include "sm_base.h"
#include "cp_mgr.h"
#include "crc32.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "phxpaxos/breakpoint.h"

namespace phxpaxos
{

CheckpointSender :: CheckpointSender(
    const nodeid_t iSendNodeID,
    Config * poConfig, 
    Learner * poLearner,
    SMFac * poSMFac, 
    CheckpointMgr * poCheckpointMgr) :
    m_iSendNodeID(iSendNodeID),
    m_poConfig(poConfig),
    m_poLearner(poLearner),
    m_poSMFac(poSMFac),
    m_poCheckpointMgr(poCheckpointMgr)
{
    m_bIsEnded = false;
    m_bIsEnd = false;
    m_bIsStarted = false;
    m_llUUID = (m_poConfig->GetMyNodeID() ^ m_poLearner->GetInstanceID()) + OtherUtils::FastRand();
    m_llSequence = 0;

    m_llAckSequence = 0;
    m_llAbsLastAckTime = 0;
}

CheckpointSender :: ~CheckpointSender()
{
}

void CheckpointSender :: Stop()
{
    if (m_bIsStarted && !m_bIsEnded)
    {
        m_bIsEnd = true;
        join();
    }
}

void CheckpointSender :: End()
{
    m_bIsEnd = true;
}

const bool CheckpointSender :: IsEnd() const
{
    return m_bIsEnded;
}

void CheckpointSender :: run()
{
    m_bIsStarted = true;
    m_llAbsLastAckTime = Time::GetSteadyClockMS();

    //pause checkpoint replayer
    bool bNeedContinue = false;
    while (!m_poCheckpointMgr->GetReplayer()->IsPaused())
    {
        if (m_bIsEnd)
        {
            m_bIsEnded = true;
            return;
        }

        bNeedContinue = true;
        
        m_poCheckpointMgr->GetReplayer()->Pause();
        PLGDebug("wait replayer paused.");
        Time::MsSleep(100);
    }

    int ret = LockCheckpoint();
    if (ret == 0)
    {
        //send
        SendCheckpoint();

        UnLockCheckpoint();
    }

    //continue checkpoint replayer
    if (bNeedContinue)
    {
        m_poCheckpointMgr->GetReplayer()->Continue();
    }

    PLGHead("Checkpoint.Sender [END]");
    m_bIsEnded = true;
}

int CheckpointSender :: LockCheckpoint()
{
    std::vector<StateMachine *> vecSMList = m_poSMFac->GetSMList();
    std::vector<StateMachine *> vecLockSMList;
    int ret = 0;
    for (auto & poSM : vecSMList)
    {
        ret = poSM->LockCheckpointState();
        if (ret != 0)
        {
            break;
        }

        vecLockSMList.push_back(poSM);
    }

    if (ret != 0)
    {
        for (auto & poSM : vecLockSMList)
        {
            poSM->UnLockCheckpointState();
        }
    }

    return ret;
}

void CheckpointSender :: UnLockCheckpoint()
{
    std::vector<StateMachine *> vecSMList = m_poSMFac->GetSMList();
    for (auto & poSM : vecSMList)
    {
        poSM->UnLockCheckpointState();
    }
}

void CheckpointSender :: SendCheckpoint()
{
    int ret = -1;
    for (int i = 0; i < 2; i++)
    {
        ret = m_poLearner->SendCheckpointBegin(
                m_iSendNodeID, m_llUUID, m_llSequence, 
                m_poSMFac->GetCheckpointInstanceID(m_poConfig->GetMyGroupIdx()));
        if (ret != 0)
        {
            PLGErr("SendCheckpointBegin fail, ret %d", ret);
            return;
        }
    }

    BP->GetCheckpointBP()->SendCheckpointBegin();

    m_llSequence++;

    std::vector<StateMachine *> vecSMList = m_poSMFac->GetSMList();
    for (auto & poSM : vecSMList)
    {
        ret = SendCheckpointFofaSM(poSM);
        if (ret != 0)
        {
            return;
        }
    }

    ret = m_poLearner->SendCheckpointEnd(
            m_iSendNodeID, m_llUUID, m_llSequence, 
            m_poSMFac->GetCheckpointInstanceID(m_poConfig->GetMyGroupIdx()));
    if (ret != 0)
    {
        PLGErr("SendCheckpointEnd fail, sequence %lu ret %d", m_llSequence, ret);
    }
    
    BP->GetCheckpointBP()->SendCheckpointEnd();
}

int CheckpointSender :: SendCheckpointFofaSM(StateMachine * poSM)
{
    string sDirPath;
    std::vector<std::string> vecFileList;

    int ret = poSM->GetCheckpointState(m_poConfig->GetMyGroupIdx(), sDirPath, vecFileList);
    if (ret != 0)
    {
        PLGErr("GetCheckpointState fail ret %d, smid %d", ret, poSM->SMID());
        return -1;
    }

    if (sDirPath.size() == 0)
    {
        PLGImp("No Checkpoint, smid %d", poSM->SMID());
        return 0;
    }

    if (sDirPath[sDirPath.size() - 1] != '/')
    {
        sDirPath += '/';
    }

    for (auto & sFilePath : vecFileList)
    {
        ret = SendFile(poSM, sDirPath, sFilePath);
        if (ret != 0)
        {
            PLGErr("SendFile fail, ret %d smid %d", ret , poSM->SMID());
            return -1;
        }
    }

    PLGImp("END, send ok, smid %d filelistcount %zu", poSM->SMID(), vecFileList.size());
    return 0;
}

int CheckpointSender :: SendFile(const StateMachine * poSM, const std::string & sDirPath, const std::string & sFilePath)
{
    PLGHead("START smid %d dirpath %s filepath %s", poSM->SMID(), sDirPath.c_str(), sFilePath.c_str());

    string sPath = sDirPath + sFilePath;

    if (m_mapAlreadySendedFile.find(sPath) != end(m_mapAlreadySendedFile))
    {
        PLGErr("file already send, filepath %s", sPath.c_str());
        return 0;
    }

    int iFD = open(sPath.c_str(), O_RDWR, S_IREAD);

    if (iFD == -1)
    {
        PLGErr("Open file fail, filepath %s", sPath.c_str());
        return -1;
    }

    ssize_t iReadLen = 0;
    size_t llOffset = 0;
    while (true)
    {
        iReadLen = read(iFD, m_sTmpBuffer, sizeof(m_sTmpBuffer));
        if (iReadLen == 0)
        {
            break;
        }

        if (iReadLen < 0)
        {
            close(iFD);
            return -1;
        }

        int ret = SendBuffer(poSM->SMID(), poSM->GetCheckpointInstanceID(m_poConfig->GetMyGroupIdx()), 
                sFilePath, llOffset, string(m_sTmpBuffer, iReadLen));
        if (ret != 0)
        {
            close(iFD);
            return ret;
        }

        PLGDebug("Send ok, offset %zu readlen %d", llOffset, iReadLen);

        if (iReadLen < (ssize_t)sizeof(m_sTmpBuffer))
        {
            break;
        }

        llOffset += iReadLen;
    }

    m_mapAlreadySendedFile[sPath] = true;

    close(iFD);
    PLGImp("END");

    return 0;
}

int CheckpointSender :: SendBuffer(const int iSMID, const uint64_t llCheckpointInstanceID, 
        const std::string & sFilePath, const uint64_t llOffset, const std::string & sBuffer)
{
    uint32_t iChecksum = crc32(0, (const uint8_t *)sBuffer.data(), sBuffer.size(), CRC32SKIP);

    int ret = 0;
    while (true)
    {
        if (m_bIsEnd)
        {
            return -1;
        }
        
        if (!CheckAck(m_llSequence))
        {
            return -1;
        }
        
        ret = m_poLearner->SendCheckpoint(
                m_iSendNodeID, m_llUUID, m_llSequence, llCheckpointInstanceID,
                iChecksum, sFilePath, iSMID, llOffset, sBuffer);

        BP->GetCheckpointBP()->SendCheckpointOneBlock();

        if (ret == 0)
        {
            m_llSequence++;
            break;
        }
        else
        {
            PLGErr("SendCheckpoint fail, ret %d need sleep 30s", ret);
            Time::MsSleep(30000);
        }
    }

    return ret;
}

void CheckpointSender :: Ack(const nodeid_t iSendNodeID, const uint64_t llUUID, const uint64_t llSequence)
{
    if (iSendNodeID != m_iSendNodeID)
    {
        PLGErr("send nodeid not same, ack.sendnodeid %lu self.sendnodeid %lu", iSendNodeID, m_iSendNodeID);
        return;
    }

    if (llUUID != m_llUUID)
    {
        PLGErr("uuid not same, ack.uuid %lu self.uuid %lu", llUUID, m_llUUID);
        return;
    }

    if (llSequence != m_llAckSequence)
    {
        PLGErr("ack_sequence not same, ack.ack_sequence %lu self.ack_sequence %lu", llSequence, m_llAckSequence);
        return;
    }

    m_llAckSequence++;
    m_llAbsLastAckTime = Time::GetSteadyClockMS();
}

const bool CheckpointSender :: CheckAck(const uint64_t llSendSequence)
{
    while (llSendSequence > m_llAckSequence + Checkpoint_ACK_LEAD)
    {
        uint64_t llNowTime = Time::GetSteadyClockMS();
        uint64_t llPassTime = llNowTime > m_llAbsLastAckTime ? llNowTime - m_llAbsLastAckTime : 0;

        if (m_bIsEnd)
        {
            return false;
        }

        if (llPassTime >= Checkpoint_ACK_TIMEOUT)
        {       
            PLGErr("Ack timeout, last acktime %lu", m_llAbsLastAckTime);
            return false;
        }       

        //PLGErr("Need sleep to slow down send speed, sendsequence %lu acksequence %lu",
                //llSendSequence, m_llAckSequence);
        Time::MsSleep(20);
    }

    return true;
}
    
}


