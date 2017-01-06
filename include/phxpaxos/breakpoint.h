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

#include <string>

namespace phxpaxos
{

class ProposerBP
{
public:
    virtual ~ProposerBP() { }
    virtual void NewProposal(const std::string & sValue) { }
    virtual void NewProposalSkipPrepare() { }
    virtual void Prepare() { }
    virtual void OnPrepareReply() { }
    virtual void OnPrepareReplyButNotPreparing() { }
    virtual void OnPrepareReplyNotSameProposalIDMsg() { }
    virtual void PreparePass(const int iUseTimeMs) { }
    virtual void PrepareNotPass() { }
    virtual void Accept() { }
    virtual void OnAcceptReply() { }
    virtual void OnAcceptReplyButNotAccepting() { }
    virtual void OnAcceptReplyNotSameProposalIDMsg() { }
    virtual void AcceptPass(const int iUseTimeMs) { }
    virtual void AcceptNotPass() { }
    virtual void PrepareTimeout() { }
    virtual void AcceptTimeout() { }
};

class AcceptorBP
{
public:
    virtual ~AcceptorBP() { }
    virtual void OnPrepare() { }
    virtual void OnPreparePass() { }
    virtual void OnPreparePersistFail() { }
    virtual void OnPrepareReject() { }
    virtual void OnAccept() { }
    virtual void OnAcceptPass() { }
    virtual void OnAcceptPersistFail() { }
    virtual void OnAcceptReject() { }
};

class LearnerBP
{
public:
    virtual ~LearnerBP() { }
    virtual void AskforLearn() { }
    virtual void OnAskforLearn() { }
    virtual void OnAskforLearnGetLockFail() { }
    virtual void SendNowInstanceID() { }
    virtual void OnSendNowInstanceID() { }
    virtual void ComfirmAskForLearn() { }
    virtual void OnComfirmAskForLearn() { }
    virtual void OnComfirmAskForLearnGetLockFail() { }
    virtual void SendLearnValue() { }
    virtual void OnSendLearnValue() { }
    virtual void SendLearnValue_Ack() { }
    virtual void OnSendLearnValue_Ack() { }
    virtual void ProposerSendSuccess() { }
    virtual void OnProposerSendSuccess() { }
    virtual void OnProposerSendSuccessNotAcceptYet() { }
    virtual void OnProposerSendSuccessBallotNotSame() { }
    virtual void OnProposerSendSuccessSuccessLearn() { }
    virtual void SenderAckTimeout() { }
    virtual void SenderAckDelay() { }
    virtual void SenderSendOnePaxosLog() { }
};

class InstanceBP
{
public:
    virtual ~InstanceBP() { }
    virtual void NewInstance() { }
    virtual void SendMessage() { }
    virtual void BroadcastMessage() { }
    virtual void OnNewValueCommitTimeout() { }
    virtual void OnReceive() { }
    virtual void OnReceiveParseError() { }
    virtual void OnReceivePaxosMsg() { }
    virtual void OnReceivePaxosMsgNodeIDNotValid() { }
    virtual void OnReceivePaxosMsgTypeNotValid() { }
    virtual void OnReceivePaxosProposerMsgInotsame() { }
    virtual void OnReceivePaxosAcceptorMsgInotsame() { }
    virtual void OnReceivePaxosAcceptorMsgAddRetry() { }
    virtual void OnInstanceLearned() { }
    virtual void OnInstanceLearnedNotMyCommit() { }
    virtual void OnInstanceLearnedIsMyCommit(const int iUseTimeMs) { }
    virtual void OnInstanceLearnedSMExecuteFail() { }
    virtual void ChecksumLogicFail() { }
};

class CommiterBP
{
public:
    virtual ~CommiterBP() { }
    virtual void NewValue() { }
    virtual void NewValueConflict() { }
    virtual void NewValueGetLockTimeout() { }
    virtual void NewValueGetLockReject() { }
    virtual void NewValueGetLockOK(const int iUseTimeMs) { }
    virtual void NewValueCommitOK(const int iUseTimeMs) { }
    virtual void NewValueCommitFail() { }

    virtual void BatchPropose() { }
    virtual void BatchProposeOK() { }
    virtual void BatchProposeFail() { }
    virtual void BatchProposeWaitTimeMs(const int iWaitTimeMs) { }
    virtual void BatchProposeDoPropose(const int iBatchCount) { }
};

class IOLoopBP
{
public:
    virtual ~IOLoopBP() { }
    virtual void OneLoop() { }
    virtual void EnqueueMsg() { }
    virtual void EnqueueMsgRejectByFullQueue() { }
    virtual void EnqueueRetryMsg() { }
    virtual void EnqueueRetryMsgRejectByFullQueue() { }
    virtual void OutQueueMsg() { }
    virtual void DealWithRetryMsg() { }
};

class NetworkBP
{
public:
    virtual ~NetworkBP() { }
    virtual void TcpEpollLoop() { }
    virtual void TcpOnError() { }
    virtual void TcpAcceptFd() { }
    virtual void TcpQueueFull() { }
    virtual void TcpReadOneMessageOk(const int iLen) { }
    virtual void TcpOnReadMessageLenError() { }
    virtual void TcpReconnect() { }
    virtual void TcpOutQueue(const int iDelayMs) { }
    virtual void SendRejectByTooLargeSize() { }
    virtual void Send(const std::string & sMessage) { }
    virtual void SendTcp(const std::string & sMessage) { }
    virtual void SendUdp(const std::string & sMessage) { }
    virtual void SendMessageNodeIDNotFound() { }
    virtual void UDPReceive(const int iRecvLen) { }
    virtual void UDPRealSend(const std::string & sMessage) { }
    virtual void UDPQueueFull() { }
};

class LogStorageBP
{
public:
    virtual ~LogStorageBP() { }
    virtual void LevelDBGetNotExist() { }
    virtual void LevelDBGetFail() { }
    virtual void FileIDToValueFail() { }
    virtual void ValueToFileIDFail() { }
    virtual void LevelDBPutFail() { }
    virtual void LevelDBPutOK(const int iUseTimeMs) { }
    virtual void AppendDataFail() { }
    virtual void AppendDataOK(const int iWriteLen, const int iUseTimeMs) { }
    virtual void GetFileChecksumNotEquel() { }
};

class AlgorithmBaseBP
{
public:
    virtual ~AlgorithmBaseBP() { }
    virtual void UnPackHeaderLenTooLong() { }
    virtual void UnPackChecksumNotSame() { }
    virtual void HeaderGidNotSame() { }
};

class CheckpointBP
{
public:
    virtual ~CheckpointBP() { }
    virtual void NeedAskforCheckpoint() { }
    virtual void SendCheckpointOneBlock() { }
    virtual void OnSendCheckpointOneBlock() { }
    virtual void SendCheckpointBegin() { }
    virtual void SendCheckpointEnd() { }
    virtual void ReceiveCheckpointDone() { }
    virtual void ReceiveCheckpointAndLoadFail() { }
    virtual void ReceiveCheckpointAndLoadSucc() { }
};

class MasterBP
{
public:
    virtual ~MasterBP() { }
    virtual void TryBeMaster() { }
    virtual void TryBeMasterProposeFail() { }
    virtual void SuccessBeMaster() { }
    virtual void OtherBeMaster() { }
    virtual void DropMaster() { }
    virtual void MasterSMInconsistent() { }
};

#define BP (Breakpoint::Instance())

class Breakpoint 
{
public:
    Breakpoint();

    virtual ~Breakpoint() { }

    void SetInstance(Breakpoint * poBreakpoint);

    static Breakpoint * Instance();
    
    virtual ProposerBP * GetProposerBP();
    
    virtual AcceptorBP * GetAcceptorBP();

    virtual LearnerBP * GetLearnerBP();

    virtual InstanceBP * GetInstanceBP();

    virtual CommiterBP * GetCommiterBP();

    virtual IOLoopBP * GetIOLoopBP();

    virtual NetworkBP * GetNetworkBP();

    virtual LogStorageBP * GetLogStorageBP();

    virtual AlgorithmBaseBP * GetAlgorithmBaseBP();

    virtual CheckpointBP * GetCheckpointBP();

    virtual MasterBP * GetMasterBP();

public:
    ProposerBP m_oProposerBP;
    AcceptorBP m_oAcceptorBP;
    LearnerBP m_oLearnerBP;
    InstanceBP m_oInstanceBP;
    CommiterBP m_oCommiterBP;
    IOLoopBP m_oIOLoopBP;
    NetworkBP m_oNetworkBP;
    LogStorageBP m_oLogStorageBP;
    AlgorithmBaseBP m_oAlgorithmBaseBP;
    CheckpointBP m_oCheckpointBP;
    MasterBP m_oMasterBP;

    static Breakpoint * m_poBreakpoint;
};
    
}
