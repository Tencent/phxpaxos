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

#include "monitor_bp.h"

namespace phxpaxos 
{

int GetKeyByUseTimeMs(const int iUseTimeMs)
{
    if (iUseTimeMs <= 10)
    {
        return 1;
    }
    else if (iUseTimeMs <= 30)
    {
        return 2;
    }
    else if (iUseTimeMs < 100)
    {
        return 3;
    }
    else if (iUseTimeMs <= 500)
    {
        return 4;
    }
    else if (iUseTimeMs <= 2000)
    {
        return 5;
    }
    else if (iUseTimeMs <= 5000)
    {
        return 6;
    }
    else if (iUseTimeMs <= 30000)
    {
        return 7;
    }
    else
    {
        return 8;
    }
}

void MonProposerBP :: NewProposal(const std::string & sValue)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 0, 1);

    if (sValue.size() < 1024)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 103, 1);
    }
    else if (sValue.size() < 5120)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 104, 1);
    }
    else if (sValue.size() < 20480)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 105, 1);
    }
    else if (sValue.size() < 102400)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 106, 1);
    }
    else if (sValue.size() < 512000)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 107, 1);
    }
    else
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 108, 1);
    }
}

void MonProposerBP :: NewProposalSkipPrepare()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 1, 1);
}

void MonProposerBP :: Prepare()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 2, 1);
}

void MonProposerBP :: OnPrepareReply()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 3, 1);
}

void MonProposerBP :: OnPrepareReplyButNotPreparing()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 4, 1);
}

void MonProposerBP :: OnPrepareReplyNotSameProposalIDMsg()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 5, 1);
}

void MonProposerBP :: PreparePass(const int iUseTimeMs)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 6, 1);

    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 18, iUseTimeMs);
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 18 + GetKeyByUseTimeMs(iUseTimeMs), 1);
}

void MonProposerBP :: PrepareNotPass()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 7, 1);
}

void MonProposerBP :: Accept()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 8, 1);
}

void MonProposerBP :: OnAcceptReply()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 9, 1);
}

void MonProposerBP :: OnAcceptReplyButNotAccepting()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 10, 1);
}

void MonProposerBP :: OnAcceptReplyNotSameProposalIDMsg()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 11, 1);
}

void MonProposerBP :: AcceptPass(const int iUseTimeMs)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 12, 1);
    
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 27, iUseTimeMs);
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 27 + GetKeyByUseTimeMs(iUseTimeMs), 1);
}

void MonProposerBP :: AcceptNotPass()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 13, 1);
}

void MonProposerBP :: PrepareTimeout()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 14, 1);
}

void MonProposerBP :: AcceptTimeout()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 15, 1);
}

////////////////////////////////////////////////////////////

void MonAcceptorBP :: OnPrepare()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 16, 1);
}

void MonAcceptorBP :: OnPreparePass()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 17, 1);
}

void MonAcceptorBP :: OnPreparePersistFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 18, 1);
}

void MonAcceptorBP :: OnPrepareReject()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 19, 1);
}

void MonAcceptorBP :: OnAccept()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 20, 1);
}

void MonAcceptorBP :: OnAcceptPass()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 21, 1);
}

void MonAcceptorBP :: OnAcceptPersistFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 22, 1);
}

void MonAcceptorBP :: OnAcceptReject()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 23, 1);
}

////////////////////////////////////////////////////
void MonLearnerBP :: AskforLearn()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 24, 1);
}

void MonLearnerBP :: OnAskforLearn()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 25, 1);
}

void MonLearnerBP :: OnAskforLearnGetLockFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 26, 1);
}

void MonLearnerBP :: SendNowInstanceID()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 27, 1);
}

void MonLearnerBP :: OnSendNowInstanceID()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 28, 1);
}

void MonLearnerBP :: ComfirmAskForLearn()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 29, 1);
}

void MonLearnerBP :: OnComfirmAskForLearn()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 30, 1);
}

void MonLearnerBP :: OnComfirmAskForLearnGetLockFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 31, 1);
}

void MonLearnerBP :: SendLearnValue()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 32, 1);
}

void MonLearnerBP :: OnSendLearnValue()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 33, 1);
}

void MonLearnerBP :: SendLearnValue_Ack()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 34, 1);
}

void MonLearnerBP :: OnSendLearnValue_Ack()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 35, 1);
}

void MonLearnerBP :: ProposerSendSuccess()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 36, 1);
}

void MonLearnerBP :: OnProposerSendSuccess()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 37, 1);
}

void MonLearnerBP :: OnProposerSendSuccessNotAcceptYet()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 38, 1);
}

void MonLearnerBP :: OnProposerSendSuccessBallotNotSame()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 39, 1);
}

void MonLearnerBP :: OnProposerSendSuccessSuccessLearn()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 40, 1);
}

void MonLearnerBP :: SenderAckTimeout()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 41, 1);
}

void MonLearnerBP :: SenderAckDelay()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 42, 1);
}

void MonLearnerBP :: SenderSendOnePaxosLog()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 43, 1);
}

///////////////////////////////////////////////////////////
void MonInstanceBP :: NewInstance()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 44, 1);
}

void MonInstanceBP :: SendMessage()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 45, 1);
}

void MonInstanceBP :: BroadcastMessage()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 46, 1);
}

void MonInstanceBP :: OnNewValueCommitTimeout()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 47, 1);
}

void MonInstanceBP :: OnReceive()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 48, 1);
}

void MonInstanceBP :: OnReceiveParseError()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 49, 1);
}

void MonInstanceBP :: OnReceivePaxosMsg()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 50, 1);
}

void MonInstanceBP :: OnReceivePaxosMsgNodeIDNotValid()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 51, 1);
}

void MonInstanceBP :: OnReceivePaxosMsgTypeNotValid()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 52, 1);
}

void MonInstanceBP :: OnReceivePaxosProposerMsgInotsame()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 53, 1);
}

void MonInstanceBP :: OnReceivePaxosAcceptorMsgInotsame()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 54, 1);
}

void MonInstanceBP :: OnReceivePaxosAcceptorMsgAddRetry()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 55, 1);
}

void MonInstanceBP :: OnInstanceLearned()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 56, 1);
}

void MonInstanceBP :: OnInstanceLearnedNotMyCommit()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 57, 1);
}

void MonInstanceBP :: OnInstanceLearnedIsMyCommit(const int iUseTimeMs)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 58, 1);

    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 0, iUseTimeMs);
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, GetKeyByUseTimeMs(iUseTimeMs), 1);
}

void MonInstanceBP :: OnInstanceLearnedSMExcuteFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 59, 1);
}

void MonInstanceBP :: ChecksumLogicFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 60, 1);
}

/////////////////////////////////////////////////////////

void MonCommiterBP :: NewValue()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 61, 1);
}

void MonCommiterBP :: NewValueConflict()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 62, 1);
}

void MonCommiterBP :: NewValueGetLockTimeout()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 63, 1);
}

void MonCommiterBP :: NewValueGetLockReject()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 109, 1);
}

void MonCommiterBP :: NewValueGetLockOK(const int iUseTimeMs)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 64, 1);

    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 56, iUseTimeMs);
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, GetKeyByUseTimeMs(iUseTimeMs) + 56, 1);
}

void MonCommiterBP :: NewValueCommitOK(const int iUseTimeMs)
{
}

void MonCommiterBP :: NewValueCommitFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 65, 1);
}

void MonCommiterBP :: BatchPropose()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 65, 1);
}

void MonCommiterBP :: BatchProposeOK()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 66, 1);
}

void MonCommiterBP :: BatchProposeFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 67, 1);
}

void MonCommiterBP :: BatchProposeWaitTimeMs(const int iWaitTimeMs)
{
    if (iWaitTimeMs <= 3)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 68, 1);
    }
    else if (iWaitTimeMs <= 10)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 69, 1);
    } 
    else if (iWaitTimeMs <= 30) 
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 70, 1);
    }
    else if (iWaitTimeMs <= 100) 
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 71, 1);
    }
    else
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 72, 1);
    }
}

void MonCommiterBP :: BatchProposeDoPropose(const int iBatchCount)
{
    if (iBatchCount <= 1)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 73, 1);
    }
    else if (iBatchCount <= 3)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 74, 1);
    }
    else if (iBatchCount <= 10)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 75, 1);
    }
    else if (iBatchCount <= 30)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 76, 1);
    }
    else if (iBatchCount <= 100)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 77, 1);
    }
    else
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 78, 1);
    }
}

//////////////////////////////////////////////////////////


void MonIOLoopBP :: OneLoop()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 66, 1);
}

void MonIOLoopBP :: EnqueueMsg()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 67, 1);
}

void MonIOLoopBP :: EnqueueMsgRejectByFullQueue()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 68, 1);
}

void MonIOLoopBP :: EnqueueRetryMsg()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 69, 1);
}

void MonIOLoopBP :: EnqueueRetryMsgRejectByFullQueue()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 70, 1);
}

void MonIOLoopBP :: OutQueueMsg()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 71, 1);
}

void MonIOLoopBP :: DealWithRetryMsg()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 72, 1);
}

////////////////////////////////////////////////////////////

void MonNetworkBP :: TcpEpollLoop()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 73, 1);
}

void MonNetworkBP :: TcpOnError()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 74, 1);
}

void MonNetworkBP :: TcpAcceptFd()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 75, 1);
}

void MonNetworkBP :: TcpQueueFull()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 76, 1);
}

void MonNetworkBP :: TcpReadOneMessageOk(const int iLen)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 77, 1);
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 78, iLen);
}

void MonNetworkBP :: TcpOnReadMessageLenError()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 79, 1);
}

void MonNetworkBP :: TcpReconnect()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 80, 1);
}

void MonNetworkBP :: TcpOutQueue(const int iDelayMs)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 79, iDelayMs);
    if (iDelayMs < 4)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 80, 1);
    }
    else if (iDelayMs < 10)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 81, 1);
    }
    else if (iDelayMs < 30)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 82, 1);
    }
    else if (iDelayMs < 100)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 83, 1);
    }
    else if (iDelayMs < 500)
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 84, 1);
    }
    else
    {
        m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 85, 1);
    }
}

void MonNetworkBP :: SendRejectByTooLargeSize()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 81, 1);
}

void MonNetworkBP :: Send(const std::string & sMessage)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 82, 1);
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 83, (int)sMessage.size());
}

void MonNetworkBP :: SendTcp(const std::string & sMessage)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 84, 1);
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 85, (int)sMessage.size());
}

void MonNetworkBP :: SendUdp(const std::string & sMessage)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 86, 1);
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 87, (int)sMessage.size());
}

void MonNetworkBP :: SendMessageNodeIDNotFound()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 88, 1);
}

void MonNetworkBP :: UDPReceive(const int iRecvLen)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 89, 1);
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 90, iRecvLen);
}

void MonNetworkBP :: UDPRealSend(const std::string & sMessage)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 91, 1);
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 92, (int)sMessage.size());
}

void MonNetworkBP :: UDPQueueFull()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 93, 1);
}

///////////////////////////////////////////////////////////////

void MonLogStorageBP :: LevelDBGetNotExist()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 94, 1);
}

void MonLogStorageBP :: LevelDBGetFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 95, 1);
}

void MonLogStorageBP :: FileIDToValueFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 96, 1);
}

void MonLogStorageBP :: ValueToFileIDFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 97, 1);
}

void MonLogStorageBP :: LevelDBPutFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 98, 1);
}

void MonLogStorageBP :: LevelDBPutOK(const int iUseTimeMs)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 36, iUseTimeMs);
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, GetKeyByUseTimeMs(iUseTimeMs) + 36, 1);
}

void MonLogStorageBP :: AppendDataFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 99, 1);
}

void MonLogStorageBP :: AppendDataOK(const int iWriteLen, const int iUseTimeMs)
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 100, 1);
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 101, iWriteLen);

    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 9, iUseTimeMs);
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 9 + GetKeyByUseTimeMs(iUseTimeMs), 1);
}

void MonLogStorageBP :: GetFileChecksumNotEquel()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iOssAttrID, 102, 1);
}

////////////////////////////////////////////////////////////////


void MonAlgorithmBaseBP :: UnPackHeaderLenTooLong()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 45, 1);
}

void MonAlgorithmBaseBP :: UnPackChecksumNotSame()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 46, 1);
}

void MonAlgorithmBaseBP :: HeaderGidNotSame()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 47, 1);
}

////////////////////////////////////////////////////////////////

void MonCheckpointBP :: NeedAskforCheckpoint()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 48, 1);
}

void MonCheckpointBP :: SendCheckpointOneBlock()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 49, 1);
}

void MonCheckpointBP :: OnSendCheckpointOneBlock()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 50, 1);
}

void MonCheckpointBP :: SendCheckpointBegin()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 51, 1);
}

void MonCheckpointBP :: SendCheckpointEnd()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 52, 1);
}

void MonCheckpointBP :: ReceiveCheckpointDone()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 53, 1);
}

void MonCheckpointBP :: ReceiveCheckpointAndLoadFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 54, 1);
}

void MonCheckpointBP :: ReceiveCheckpointAndLoadSucc()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 55, 1);
}

/////////////////////////////////////////////////////////////////

void MonMasterBP :: TryBeMaster()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 86, 1);
}

void MonMasterBP :: TryBeMasterProposeFail()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 87, 1);
}

void MonMasterBP :: SuccessBeMaster()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 88, 1);
}

void MonMasterBP :: OtherBeMaster()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 89, 1);
}

void MonMasterBP :: DropMaster()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 90, 1);
}

void MonMasterBP :: MasterSMInconsistent()
{
    m_pIDKeyOssFunc(m_oMonitorConfig.iUseTimeOssAttrID, 91, 1);
}

/////////////////////////////////////////////////////////////////

MonitorBP :: MonitorBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) : 
    m_oProposerBP(oMonitorConfig, pIDKeyOssFunc), m_oAcceptorBP(oMonitorConfig, pIDKeyOssFunc),
    m_oLearnerBP(oMonitorConfig, pIDKeyOssFunc), m_oInstanceBP(oMonitorConfig, pIDKeyOssFunc),
    m_oCommiterBP(oMonitorConfig, pIDKeyOssFunc), m_oIOLoopBP(oMonitorConfig, pIDKeyOssFunc),
    m_oNetworkBP(oMonitorConfig, pIDKeyOssFunc), m_oLogStorageBP(oMonitorConfig, pIDKeyOssFunc),
    m_oAlgorithmBaseBP(oMonitorConfig, pIDKeyOssFunc), m_oCheckpointBP(oMonitorConfig, pIDKeyOssFunc),
    m_oMasterBP(oMonitorConfig, pIDKeyOssFunc)
{
}

ProposerBP * MonitorBP :: GetProposerBP()
{
    return &m_oProposerBP;
}

AcceptorBP * MonitorBP :: GetAcceptorBP()
{
    return &m_oAcceptorBP;
}

LearnerBP * MonitorBP :: GetLearnerBP()
{
    return &m_oLearnerBP;
}

InstanceBP * MonitorBP :: GetInstanceBP()
{
    return &m_oInstanceBP;
}

CommiterBP * MonitorBP :: GetCommiterBP()
{
    return &m_oCommiterBP;
}

IOLoopBP * MonitorBP :: GetIOLoopBP()
{
    return &m_oIOLoopBP;
}

NetworkBP * MonitorBP :: GetNetworkBP()
{
    return &m_oNetworkBP;
}

LogStorageBP * MonitorBP :: GetLogStorageBP()
{
    return &m_oLogStorageBP;
}

AlgorithmBaseBP * MonitorBP :: GetAlgorithmBaseBP()
{
    return &m_oAlgorithmBaseBP;
}

CheckpointBP *  MonitorBP :: GetCheckpointBP()
{
    return &m_oCheckpointBP;
}

MasterBP * MonitorBP :: GetMasterBP()
{
    return &m_oMasterBP;
}
    
}


