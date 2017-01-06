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

#include "phxpaxos/breakpoint.h"
#include <stdio.h>
#include "phxpaxos_plugin/monitor.h"

namespace phxpaxos 
{

class MonProposerBP : public ProposerBP
{
public:
    MonProposerBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void NewProposal(const std::string & sValue);
    void NewProposalSkipPrepare();
    void Prepare();
    void OnPrepareReply();
    void OnPrepareReplyButNotPreparing();
    void OnPrepareReplyNotSameProposalIDMsg();
    void PreparePass(const int iUseTimeMs);
    void PrepareNotPass();
    void Accept();
    void OnAcceptReply();
    void OnAcceptReplyButNotAccepting();
    void OnAcceptReplyNotSameProposalIDMsg();
    void AcceptPass(const int iUseTimeMs);
    void AcceptNotPass();
    void PrepareTimeout();
    void AcceptTimeout();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

class MonAcceptorBP : public AcceptorBP
{
public:
    MonAcceptorBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void OnPrepare();
    void OnPreparePass();
    void OnPreparePersistFail();
    void OnPrepareReject();
    void OnAccept();
    void OnAcceptPass();
    void OnAcceptPersistFail();
    void OnAcceptReject();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

class MonLearnerBP : public LearnerBP
{
public:
    MonLearnerBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void AskforLearn();
    void OnAskforLearn();
    void OnAskforLearnGetLockFail();
    void SendNowInstanceID();
    void OnSendNowInstanceID();
    void ComfirmAskForLearn();
    void OnComfirmAskForLearn();
    void OnComfirmAskForLearnGetLockFail();
    void SendLearnValue();
    void OnSendLearnValue();
    void SendLearnValue_Ack();
    void OnSendLearnValue_Ack();
    void ProposerSendSuccess();
    void OnProposerSendSuccess();
    void OnProposerSendSuccessNotAcceptYet();
    void OnProposerSendSuccessBallotNotSame();
    void OnProposerSendSuccessSuccessLearn();
    void SenderAckTimeout();
    void SenderAckDelay();
    void SenderSendOnePaxosLog();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

class MonInstanceBP : public InstanceBP
{
public:
    MonInstanceBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void NewInstance();
    void SendMessage();
    void BroadcastMessage();
    void OnNewValueCommitTimeout();
    void OnReceive();
    void OnReceiveParseError();
    void OnReceivePaxosMsg();
    void OnReceivePaxosMsgNodeIDNotValid();
    void OnReceivePaxosMsgTypeNotValid();
    void OnReceivePaxosProposerMsgInotsame();
    void OnReceivePaxosAcceptorMsgInotsame();
    void OnReceivePaxosAcceptorMsgAddRetry();
    void OnInstanceLearned();
    void OnInstanceLearnedNotMyCommit();
    void OnInstanceLearnedIsMyCommit(const int iUseTimeMs);
    void OnInstanceLearnedSMExcuteFail();
    void ChecksumLogicFail();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

class MonCommiterBP : public CommiterBP
{
public:
    MonCommiterBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void NewValue();
    void NewValueConflict();
    void NewValueGetLockTimeout();
    void NewValueGetLockReject();
    void NewValueGetLockOK(const int iUseTimeMs);
    void NewValueCommitOK(const int iUseTimeMs);
    void NewValueCommitFail();

    void BatchPropose();
    void BatchProposeOK();
    void BatchProposeFail();
    void BatchProposeWaitTimeMs(const int iWaitTimeMs);
    void BatchProposeDoPropose(const int iBatchCount);

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

class MonIOLoopBP : public IOLoopBP
{
public:
    MonIOLoopBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void OneLoop();
    void EnqueueMsg();
    void EnqueueMsgRejectByFullQueue();
    void EnqueueRetryMsg();
    void EnqueueRetryMsgRejectByFullQueue();
    void OutQueueMsg();
    void DealWithRetryMsg();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

class MonNetworkBP : public NetworkBP
{
public:
    MonNetworkBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void TcpEpollLoop();
    void TcpOnError();
    void TcpAcceptFd();
    void TcpQueueFull();
    void TcpReadOneMessageOk(const int iLen);
    void TcpOnReadMessageLenError();
    void TcpReconnect();
    void TcpOutQueue(const int iDelayMs);
    void SendRejectByTooLargeSize();
    void Send(const std::string & sMessage);
    void SendTcp(const std::string & sMessage);
    void SendUdp(const std::string & sMessage);
    void SendMessageNodeIDNotFound();
    void UDPReceive(const int iRecvLen);
    void UDPRealSend(const std::string & sMessage);
    void UDPQueueFull();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

class MonLogStorageBP : public LogStorageBP
{
public:
    MonLogStorageBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }
    void LevelDBGetNotExist();
    void LevelDBGetFail();
    void FileIDToValueFail();
    void ValueToFileIDFail();
    void LevelDBPutFail();
    void LevelDBPutOK(const int iUseTimeMs);
    void AppendDataFail();
    void AppendDataOK(const int iWriteLen, const int iUseTimeMs);
    void GetFileChecksumNotEquel();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

///////////////////////////////////////////////////////////////

class MonAlgorithmBaseBP : public AlgorithmBaseBP
{
public:
    MonAlgorithmBaseBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }

    virtual void UnPackHeaderLenTooLong();
    virtual void UnPackChecksumNotSame();
    virtual void HeaderGidNotSame();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

///////////////////////////////////////////////////////////////

class MonCheckpointBP : public CheckpointBP
{
public:
    MonCheckpointBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }

    virtual void NeedAskforCheckpoint();
    virtual void SendCheckpointOneBlock();
    virtual void OnSendCheckpointOneBlock();
    virtual void SendCheckpointBegin();
    virtual void SendCheckpointEnd();
    virtual void ReceiveCheckpointDone();
    virtual void ReceiveCheckpointAndLoadFail();
    virtual void ReceiveCheckpointAndLoadSucc();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};


///////////////////////////////////////////////////////////////

class MonMasterBP : public MasterBP
{
public:
    MonMasterBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc) 
        : m_oMonitorConfig(oMonitorConfig), m_pIDKeyOssFunc(pIDKeyOssFunc) { }

    virtual void TryBeMaster();
    virtual void TryBeMasterProposeFail();
    virtual void SuccessBeMaster();
    virtual void OtherBeMaster();
    virtual void DropMaster();
    virtual void MasterSMInconsistent();

private:
    MonitorConfig m_oMonitorConfig;
    IDKeyOssFunc m_pIDKeyOssFunc;
};

///////////////////////////////////////////////////////////////

class MonitorBP : public Breakpoint
{
public:
    MonitorBP(const MonitorConfig & oMonitorConfig, IDKeyOssFunc pIDKeyOssFunc);

    ProposerBP * GetProposerBP();
    
    AcceptorBP * GetAcceptorBP();

    LearnerBP * GetLearnerBP();

    InstanceBP * GetInstanceBP();

    CommiterBP * GetCommiterBP();

    IOLoopBP * GetIOLoopBP();

    NetworkBP * GetNetworkBP();

    LogStorageBP * GetLogStorageBP();

    AlgorithmBaseBP * GetAlgorithmBaseBP();

    CheckpointBP * GetCheckpointBP();

    MasterBP * GetMasterBP();

public:
    MonProposerBP m_oProposerBP;
    MonAcceptorBP m_oAcceptorBP;
    MonLearnerBP m_oLearnerBP;
    MonInstanceBP m_oInstanceBP;
    MonCommiterBP m_oCommiterBP;
    MonIOLoopBP m_oIOLoopBP;
    MonNetworkBP m_oNetworkBP;
    MonLogStorageBP m_oLogStorageBP;
    MonAlgorithmBaseBP m_oAlgorithmBaseBP;
    MonCheckpointBP m_oCheckpointBP;
    MonMasterBP m_oMasterBP;
};
    
}
