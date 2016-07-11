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

#include "instance.h"
#include "proposer.h"
#include "acceptor.h"
#include "learner.h"

namespace phxpaxos
{

Instance :: Instance(
        const Config * poConfig, 
        const LogStorage * poLogStorage,
        const MsgTransport * poMsgTransport,
        const bool bUseCheckpointReplayer)
    : m_oIOLoop((Config *)poConfig, this),
    m_oAcceptor(poConfig, poMsgTransport, this, poLogStorage), 
    m_oLearner(poConfig, poMsgTransport, this, &m_oAcceptor, poLogStorage, &m_oIOLoop, &m_oCheckpointMgr, &m_oSMFac),
    m_oProposer(poConfig, poMsgTransport, this, &m_oLearner, &m_oIOLoop),
    m_oPaxosLog(poLogStorage),
    m_oCommitCtx((Config *)poConfig),
    m_oCommitter((Config *)poConfig, &m_oCommitCtx, &m_oIOLoop, &m_oSMFac),
    m_oCheckpointMgr((Config *)poConfig, &m_oSMFac, (LogStorage *)poLogStorage, bUseCheckpointReplayer)
{
    m_poConfig = (Config *)poConfig;
    m_poMsgTransport = (MsgTransport *)poMsgTransport;
    m_iCommitTimerID = 0;
    m_iLastChecksum = 0;
}

Instance :: ~Instance()
{
    m_oIOLoop.Stop();
    m_oCheckpointMgr.Stop();
    m_oLearner.Stop();

    PLGHead("Instance Deleted, GroupIdx %d.", m_poConfig->GetMyGroupIdx());
}

int Instance :: Init()
{
    m_oLearner.Init();

    //Must init acceptor first, because the max instanceid is record in acceptor state.
    int ret = m_oAcceptor.Init();
    if (ret != 0)
    {
        PLGErr("Acceptor.Init fail, ret %d", ret);
        return ret;
    }

    ret = m_oCheckpointMgr.Init();
    if (ret != 0)
    {
        PLGErr("CheckpointMgr.Init fail, ret %d", ret);
        return ret;
    }

    uint64_t llCPInstanceID = m_oCheckpointMgr.GetCheckpointInstanceID() + 1;

    PLGImp("Acceptor.OK, Log.InstanceID %lu Checkpoint.InstanceID %lu", 
            m_oAcceptor.GetInstanceID(), llCPInstanceID);

    uint64_t llNowInstanceID = llCPInstanceID;
    if (llNowInstanceID < m_oAcceptor.GetInstanceID())
    {
        ret = PlayLog(llNowInstanceID, m_oAcceptor.GetInstanceID());
        if (ret != 0)
        {
            return ret;
        }

        PLGImp("PlayLog OK, begin instanceid %lu end instanceid %lu", llNowInstanceID, m_oAcceptor.GetInstanceID());

        llNowInstanceID = m_oAcceptor.GetInstanceID();
    }
    else
    {
        if (llNowInstanceID > m_oAcceptor.GetInstanceID())
        {
            m_oAcceptor.InitForNewPaxosInstance();
        }
        
        m_oAcceptor.SetInstanceID(llNowInstanceID);
    }

    PLGImp("NowInstanceID %lu", llNowInstanceID);

    m_oLearner.SetInstanceID(llNowInstanceID);
    m_oProposer.SetInstanceID(llNowInstanceID);
    m_oProposer.SetStartProposalID(m_oAcceptor.GetAcceptorState()->GetPromiseBallot().m_llProposalID + 1);

    m_oCheckpointMgr.SetMaxChosenInstanceID(llNowInstanceID);

    ret = InitLastCheckSum();
    if (ret != 0)
    {
        return ret;
    }

    m_oLearner.Reset_AskforLearn_Noop();

    //start ioloop
    m_oIOLoop.start();
    //start checkpoint replayer and cleaner
    m_oCheckpointMgr.Start();

    PLGImp("OK");

    return 0;
}

int Instance :: InitLastCheckSum()
{
    if (m_oAcceptor.GetInstanceID() == 0)
    {
        m_iLastChecksum = 0;
        return 0;
    }

    if (m_oAcceptor.GetInstanceID() <= m_oCheckpointMgr.GetMinChosenInstanceID())
    {
        m_iLastChecksum = 0;
        return 0;
    }

    AcceptorStateData oState;
    int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), m_oAcceptor.GetInstanceID() - 1, oState);
    if (ret != 0 && ret != 1)
    {
        return ret;
    }

    if (ret == 1)
    {
        PLGErr("las checksum not exist, now instanceid %lu", m_oAcceptor.GetInstanceID());
        m_iLastChecksum = 0;
        return 0;
    }

    m_iLastChecksum = oState.checksum();

    PLGImp("ok, last checksum %u", m_iLastChecksum);

    return 0;
}

int Instance :: PlayLog(const uint64_t llBeginInstanceID, const uint64_t llEndInstanceID)
{
    if (llBeginInstanceID < m_oCheckpointMgr.GetMinChosenInstanceID())
    {
        PLGErr("now instanceid %lu small than min chosen instanceid %lu", 
                llBeginInstanceID, m_oCheckpointMgr.GetMinChosenInstanceID());
        return -2;
    }

    for (uint64_t llInstanceID = llBeginInstanceID; llInstanceID < llEndInstanceID; llInstanceID++)
    {
        AcceptorStateData oState; 
        int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), llInstanceID, oState);
        if (ret != 0)
        {
            PLGErr("log read fail, instanceid %lu ret %d", llInstanceID, ret);
            return ret;
        }

        bool bExecuteRet = m_oSMFac.Execute(m_poConfig->GetMyGroupIdx(), llInstanceID, oState.acceptedvalue(), nullptr);
        if (!bExecuteRet)
        {
            PLGErr("Execute fail, instanceid %lu", llInstanceID);
            return -1;
        }
    }

    return 0;
}

const uint32_t Instance :: GetLastChecksum()
{
    return m_iLastChecksum;
}

Committer * Instance :: GetCommitter()
{
    return &m_oCommitter;
}

Cleaner * Instance :: GetCheckpointCleaner()
{
    return m_oCheckpointMgr.GetCleaner();
}

Replayer * Instance :: GetCheckpointReplayer()
{
    return m_oCheckpointMgr.GetReplayer();
}

////////////////////////////////////////////////

void Instance :: CheckNewValue()
{
    if (!m_oCommitCtx.IsNewCommit())
    {
        return;
    }

    if (!m_oLearner.IsIMLatest())
    {
        return;
    }

    if (m_poConfig->IsIMFollower())
    {
        PLGErr("I'm follower, skip this new value");
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Follower_Cannot_Commit);
        return;
    }

    if (!m_poConfig->CheckConfig())
    {
        PLGErr("I'm not in membership, skip this new value");
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Im_Not_In_Membership);
        return;
    }

    if ((int)m_oCommitCtx.GetCommitValue().size() > MAX_VALUE_SIZE)
    {
        PLGErr("value size %zu to large, skip this new value",
            m_oCommitCtx.GetCommitValue().size());
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Value_Size_TooLarge);
        return;
    }

    m_oCommitCtx.StartCommit(m_oProposer.GetInstanceID());

    if (m_oCommitCtx.GetTimeoutMs() != -1)
    {
        m_oIOLoop.AddTimer(m_oCommitCtx.GetTimeoutMs(), Timer_Instance_Commit_Timeout, m_iCommitTimerID);
    }
    
    m_oTimeStat.Point();

    if (m_poConfig->GetIsUseMembership()
            && (m_oProposer.GetInstanceID() == 0 || m_poConfig->GetGid() == 0))
    {
        //Init system variables.
        PLGHead("Need to init system variables, Now.InstanceID %lu Now.Gid %lu", 
                m_oProposer.GetInstanceID(), m_poConfig->GetGid());

        uint64_t llGid = OtherUtils::GenGid(m_poConfig->GetMyNodeID());
        string sInitSVOpValue;
        int ret = m_poConfig->GetSystemVSM()->CreateGid_OPValue(llGid, sInitSVOpValue);
        assert(ret == 0);

        m_oSMFac.PackPaxosValue(sInitSVOpValue, m_poConfig->GetSystemVSM()->SMID());
        m_oProposer.NewValue(sInitSVOpValue);
    }
    else
    {
        m_oProposer.NewValue(m_oCommitCtx.GetCommitValue());
    }
}

void Instance :: OnNewValueCommitTimeout()
{
    BP->GetInstanceBP()->OnNewValueCommitTimeout();

    m_oProposer.ExitPrepare();
    m_oProposer.ExitAccept();

    m_oCommitCtx.SetResult(PaxosTryCommitRet_Timeout, m_oProposer.GetInstanceID(), "");
}

//////////////////////////////////////////////////////////////////////

int Instance :: OnReceiveMessage(const char * pcMessage, const int iMessageLen)
{
    m_oIOLoop.AddMessage(pcMessage, iMessageLen);

    return 0;
}

bool Instance :: ReceiveMsgHeaderCheck(const Header & oHeader, const nodeid_t iFromNodeID)
{
    if (m_poConfig->GetGid() == 0 || oHeader.gid() == 0)
    {
        return true;
    }

    if (m_poConfig->GetGid() != oHeader.gid())
    {
        BP->GetAlgorithmBaseBP()->HeaderGidNotSame();
        PLGErr("Header check fail, header.gid %lu config.gid %lu, msg.from_nodeid %lu",
                oHeader.gid(), m_poConfig->GetGid(), iFromNodeID);
        return false;
    }

    return true;
}

void Instance :: OnReceive(const std::string & sBuffer)
{
    BP->GetInstanceBP()->OnReceive();

    if (sBuffer.size() <= 6)
    {
        PLGErr("buffer size %zu too short", sBuffer.size());
        return;
    }

    Header oHeader;
    size_t iBodyStartPos = 0;
    size_t iBodyLen = 0;
    int ret = Base::UnPackBaseMsg(sBuffer, oHeader, iBodyStartPos, iBodyLen);
    if (ret != 0)
    {
        return;
    }

    int iCmd = oHeader.cmdid();

    if (iCmd == MsgCmd_PaxosMsg)
    {
        if (m_oCheckpointMgr.InAskforcheckpointMode())
        {
            PLGImp("in ask for checkpoint mode, ignord paxosmsg");
            return;
        }
        
        PaxosMsg oPaxosMsg;
        bool bSucc = oPaxosMsg.ParseFromArray(sBuffer.data() + iBodyStartPos, iBodyLen);
        if (!bSucc)
        {
            BP->GetInstanceBP()->OnReceiveParseError();
            PLGErr("PaxosMsg.ParseFromArray fail, skip this msg");
            return;
        }

        if (!ReceiveMsgHeaderCheck(oHeader, oPaxosMsg.nodeid()))
        {
            return;
        }
        
        OnReceivePaxosMsg(oPaxosMsg);
    }
    else if (iCmd == MsgCmd_CheckpointMsg)
    {
        CheckpointMsg oCheckpointMsg;
        bool bSucc = oCheckpointMsg.ParseFromArray(sBuffer.data() + iBodyStartPos, iBodyLen);
        if (!bSucc)
        {
            BP->GetInstanceBP()->OnReceiveParseError();
            PLGErr("PaxosMsg.ParseFromArray fail, skip this msg");
            return;
        }

        if (!ReceiveMsgHeaderCheck(oHeader, oCheckpointMsg.nodeid()))
        {
            return;
        }
        
        OnReceiveCheckpointMsg(oCheckpointMsg);
    }
}

void Instance :: OnReceiveCheckpointMsg(const CheckpointMsg & oCheckpointMsg)
{
    PLGImp("Now.InstanceID %lu MsgType %d Msg.from_nodeid %lu My.nodeid %lu flag %d"
            " uuid %lu sequence %lu checksum %lu offset %lu buffsize %zu filepath %s",
            m_oAcceptor.GetInstanceID(), oCheckpointMsg.msgtype(), oCheckpointMsg.nodeid(),
            m_poConfig->GetMyNodeID(), oCheckpointMsg.flag(), oCheckpointMsg.uuid(), oCheckpointMsg.sequence(), oCheckpointMsg.checksum(),
            oCheckpointMsg.offset(), oCheckpointMsg.buffer().size(), oCheckpointMsg.filepath().c_str());

    if (oCheckpointMsg.msgtype() == CheckpointMsgType_SendFile)
    {
        if (!m_oCheckpointMgr.InAskforcheckpointMode())
        {
            PLGImp("not in ask for checkpoint mode, ignord checkpoint msg");
            return;
        }

        m_oLearner.OnSendCheckpoint(oCheckpointMsg);
    }
    else if (oCheckpointMsg.msgtype() == CheckpointMsgType_SendFile_Ack)
    {
        m_oLearner.OnSendCheckpointAck(oCheckpointMsg);
    }
}

int Instance :: OnReceivePaxosMsg(const PaxosMsg & oPaxosMsg)
{
    BP->GetInstanceBP()->OnReceivePaxosMsg();

    PLGImp("Now.InstanceID %lu Msg.InstanceID %lu MsgType %d Msg.from_nodeid %lu My.nodeid %lu Seen.LatestInstanceID %lu",
            m_oProposer.GetInstanceID(), oPaxosMsg.instanceid(), oPaxosMsg.msgtype(),
            oPaxosMsg.nodeid(), m_poConfig->GetMyNodeID(), m_oLearner.GetSeenLatestInstanceID());

    if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply
            || oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply
            || oPaxosMsg.msgtype() == MsgType_PaxosProposal_SendNewValue)
    {
        if (!m_poConfig->IsValidNodeID(oPaxosMsg.nodeid()))
        {
            BP->GetInstanceBP()->OnReceivePaxosMsgNodeIDNotValid();
            PLGErr("acceptor reply type msg, from nodeid not in my membership, skip this message");
            return 0;
        }
        
        return ReceiveMsgForProposer(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosPrepare
            || oPaxosMsg.msgtype() == MsgType_PaxosAccept)
    {
        //if my gid is zero, then this is a unknown node.
        if (m_poConfig->GetGid() == 0)
        {
            m_poConfig->AddTmpNodeOnlyForLearn(oPaxosMsg.nodeid());
        }
        
        if ((!m_poConfig->IsValidNodeID(oPaxosMsg.nodeid())))
        {
            PLGErr("prepare/accept type msg, from nodeid not in my membership(or i'm null membership), "
                    "skip this message and add node to tempnode, my gid %lu",
                    m_poConfig->GetGid());

            m_poConfig->AddTmpNodeOnlyForLearn(oPaxosMsg.nodeid());

            return 0;
        }

        ChecksumLogic(oPaxosMsg);
        return ReceiveMsgForAcceptor(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforLearn
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_ProposerSendSuccess
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_ComfirmAskforLearn
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendNowInstanceID
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue_Ack
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforCheckpoint)
    {
        ChecksumLogic(oPaxosMsg);
        return ReceiveMsgForLearner(oPaxosMsg);
    }
    else
    {
        BP->GetInstanceBP()->OnReceivePaxosMsgTypeNotValid();
        PLGErr("Invaid msgtype %d", oPaxosMsg.msgtype());
    }

    return 0;
}

int Instance :: ReceiveMsgForProposer(const PaxosMsg & oPaxosMsg)
{
    if (m_poConfig->IsIMFollower())
    {
        PLGErr("I'm follower, skip this message");
        return 0;
    }

    ///////////////////////////////////////////////////////////////
    
    if (oPaxosMsg.instanceid() != m_oProposer.GetInstanceID())
    {
        BP->GetInstanceBP()->OnReceivePaxosProposerMsgInotsame();
        PLGErr("InstanceID not same, skip msg");
        return 0;
    }

    if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply)
    {
        m_oProposer.OnPrepareReply(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply)
    {
        m_oProposer.OnAcceptReply(oPaxosMsg);
    }

    return 0;
}

int Instance :: ReceiveMsgForAcceptor(const PaxosMsg & oPaxosMsg)
{
    if (m_poConfig->IsIMFollower())
    {
        PLGErr("I'm follower, skip this message");
        return 0;
    }
    
    //////////////////////////////////////////////////////////////
    
    if (oPaxosMsg.instanceid() != m_oAcceptor.GetInstanceID())
    {
        BP->GetInstanceBP()->OnReceivePaxosAcceptorMsgInotsame();
    }
    
    if (oPaxosMsg.instanceid() == m_oAcceptor.GetInstanceID() + 1)
    {
        //skip success message
        PaxosMsg oNewPaxosMsg = oPaxosMsg;
        oNewPaxosMsg.set_instanceid(m_oAcceptor.GetInstanceID());
        oNewPaxosMsg.set_msgtype(MsgType_PaxosLearner_ProposerSendSuccess);

        ReceiveMsgForLearner(oNewPaxosMsg);
    }
            
    if (oPaxosMsg.instanceid() == m_oAcceptor.GetInstanceID())
    {
        if (oPaxosMsg.msgtype() == MsgType_PaxosPrepare)
        {
            return m_oAcceptor.OnPrepare(oPaxosMsg);
        }
        else if (oPaxosMsg.msgtype() == MsgType_PaxosAccept)
        {
            m_oAcceptor.OnAccept(oPaxosMsg);
        }
    }
    else if (oPaxosMsg.instanceid() > m_oAcceptor.GetInstanceID())
    {
        if (oPaxosMsg.instanceid() >= m_oLearner.GetSeenLatestInstanceID())
        {
            //need retry msg precondition
            //1. prepare or accept msg
            //2. msg.instanceid > nowinstanceid. 
            //    (if < nowinstanceid, this msg is expire)
            //3. msg.instanceid >= seen latestinstanceid. 
            //    (if < seen latestinstanceid, proposer don't need reply with this instanceid anymore.)
            m_oIOLoop.AddRetryPaxosMsg(oPaxosMsg);
            
            BP->GetInstanceBP()->OnReceivePaxosAcceptorMsgAddRetry();

            PLGErr("InstanceID not same, get in to retry logic");
        }
    }

    return 0;
}

int Instance :: ReceiveMsgForLearner(const PaxosMsg & oPaxosMsg)
{
    if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforLearn)
    {
        m_oLearner.OnAskforLearn(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue)
    {
        m_oLearner.OnSendLearnValue(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_ProposerSendSuccess)
    {
        m_oLearner.OnProposerSendSuccess(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendNowInstanceID)
    {
        m_oLearner.OnSendNowInstanceID(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_ComfirmAskforLearn)
    {
        m_oLearner.OnComfirmAskForLearn(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue_Ack)
    {
        m_oLearner.OnSendLearnValue_Ack(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforCheckpoint)
    {
        m_oLearner.OnAskforCheckpoint(oPaxosMsg);
    }

    if (m_oLearner.IsLearned())
    {
        BP->GetInstanceBP()->OnInstanceLearned();

        StateMachine * poSM = nullptr;
        SMCtx * poSMCtx = nullptr;
        bool bIsMyCommit = m_oCommitCtx.IsMyCommit(m_oLearner.GetInstanceID(), m_oLearner.GetLearnValue(), poSM, poSMCtx);

        if (!bIsMyCommit)
        {
            BP->GetInstanceBP()->OnInstanceLearnedNotMyCommit();
            PLGDebug("this value is not my commit");
        }
        else
        {
            int iUseTimeMs = m_oTimeStat.Point();
            BP->GetInstanceBP()->OnInstanceLearnedIsMyCommit(iUseTimeMs);
            PLGHead("My commit ok, usetime %dms", iUseTimeMs);
        }

        if (!SMExecute(m_oLearner.GetInstanceID(), m_oLearner.GetLearnValue(), bIsMyCommit, poSM, poSMCtx))
        {
            BP->GetInstanceBP()->OnInstanceLearnedSMExecuteFail();

            PLGErr("SMExecute fail, instanceid %lu, not increase instanceid", m_oLearner.GetInstanceID());
            m_oCommitCtx.SetResult(PaxosTryCommitRet_ExecuteFail, 
                    m_oLearner.GetInstanceID(), m_oLearner.GetLearnValue());

            m_oProposer.CancelSkipPrepare();

            return -1;
        }
        
        {
            //this paxos instance end, tell proposal done
            m_oCommitCtx.SetResult(PaxosTryCommitRet_OK
                    , m_oLearner.GetInstanceID(), m_oLearner.GetLearnValue());

            if (m_iCommitTimerID > 0)
            {
                m_oIOLoop.RemoveTimer(m_iCommitTimerID);
            }
        }
        
        PLGHead("[Learned] New paxos starting, Now.Proposer.InstanceID %lu "
                "Now.Acceptor.InstanceID %lu Now.Learner.InstanceID %lu",
                m_oProposer.GetInstanceID(), m_oAcceptor.GetInstanceID(), m_oLearner.GetInstanceID());
        
        PLGHead("[Learned] Checksum change, last checksum %u new checksum %u",
                m_iLastChecksum, m_oLearner.GetNewChecksum());

        m_iLastChecksum = m_oLearner.GetNewChecksum();

        NewInstance();

        PLGHead("[Learned] New paxos instance has started, Now.Proposer.InstanceID %lu "
                "Now.Acceptor.InstanceID %lu Now.Learner.InstanceID %lu",
                m_oProposer.GetInstanceID(), m_oAcceptor.GetInstanceID(), m_oLearner.GetInstanceID());

        m_oCheckpointMgr.SetMaxChosenInstanceID(m_oAcceptor.GetInstanceID());

        BP->GetInstanceBP()->NewInstance();
    }

    return 0;
}

void Instance :: NewInstance()
{
    m_oAcceptor.NewInstance();
    m_oLearner.NewInstance();
    m_oProposer.NewInstance();
}

const uint64_t Instance :: GetNowInstanceID()
{
    return m_oAcceptor.GetInstanceID();
}

///////////////////////////////

void Instance :: OnTimeout(const uint32_t iTimerID, const int iType)
{
    if (iType == Timer_Proposer_Prepare_Timeout)
    {
        m_oProposer.OnPrepareTimeout();
    }
    else if (iType == Timer_Proposer_Accept_Timeout)
    {
        m_oProposer.OnAcceptTimeout();
    }
    else if (iType == Timer_Learner_Askforlearn_noop)
    {
        m_oLearner.AskforLearn_Noop();
    }
    else if (iType == Timer_Instance_Commit_Timeout)
    {
        OnNewValueCommitTimeout();
    }
    else
    {
        PLGErr("unknown timer type %d, timerid %u", iType, iTimerID);
    }
}

////////////////////////////////

void Instance :: AddStateMachine(StateMachine * poSM)
{
    m_oSMFac.AddSM(poSM);
}

bool Instance :: SMExecute(
        const uint64_t llInstanceID, 
        const std::string & sValue, 
        const bool bIsMyCommit,
        StateMachine * poSM,
        SMCtx * poSMCtx)
{
    if (bIsMyCommit && poSM != nullptr)
    {
        return m_oSMFac.DoExecute(poSM, m_poConfig->GetMyGroupIdx(), llInstanceID, sValue, poSMCtx);
    }
    else 
    {
        return m_oSMFac.Execute(m_poConfig->GetMyGroupIdx(), llInstanceID, sValue, poSMCtx);
    }
}

////////////////////////////////

void Instance :: ChecksumLogic(const PaxosMsg & oPaxosMsg)
{
    if (oPaxosMsg.lastchecksum() == 0)
    {
        return;
    }

    if (oPaxosMsg.instanceid() != m_oAcceptor.GetInstanceID())
    {
        return;
    }

    if (m_oAcceptor.GetInstanceID() > 0 && GetLastChecksum() == 0)
    {
        PLGErr("I have no last checksum, other last checksum %u", oPaxosMsg.lastchecksum());
        m_iLastChecksum = oPaxosMsg.lastchecksum();
        return;
    }
    
    PLGHead("my last checksum %u other last checksum %u", GetLastChecksum(), oPaxosMsg.lastchecksum());

    if (oPaxosMsg.lastchecksum() != GetLastChecksum())
    {
        PLGErr("checksum fail, my last checksum %u other last checksum %u", 
                 GetLastChecksum(), oPaxosMsg.lastchecksum());
        BP->GetInstanceBP()->ChecksumLogicFail();
    }

    assert(oPaxosMsg.lastchecksum() == GetLastChecksum());
}

}


