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

#include "pnode.h"

namespace phxpaxos
{

PNode :: PNode()
    : m_iMyNodeID(nullnode)
{
}

PNode :: ~PNode()
{
    //1.step: must stop master(app) first.
    for (auto & poMaster : m_vecMasterList)
    {
        poMaster->StopMaster();
    }

    //2.step: stop network.
    m_oDefaultNetWork.StopNetWork();

    //3.step: delete paxos instance.
    for (auto & poGroup : m_vecGroupList)
    {
        delete poGroup;
    }

    //4. step: delete master state machine.
    for (auto & poMaster : m_vecMasterList)
    {
        delete poMaster;
    }
}

int PNode :: InitLogStorage(const Options & oOptions, LogStorage *& poLogStorage)
{
    if (oOptions.poLogStorage != nullptr)
    {
        poLogStorage = oOptions.poLogStorage;
        PLImp("OK, use user logstorage");
        return 0;
    }

    if (oOptions.sLogStoragePath.size() == 0)
    {
        PLErr("LogStorage Path is null");
        return -2;
    }

    int ret = m_oDefaultLogStorage.Init(oOptions.sLogStoragePath, oOptions.iGroupCount);
    if (ret != 0)
    {
        PLErr("Init default logstorage fail, logpath %s ret %d",
                oOptions.sLogStoragePath.c_str(), ret);
        return ret;
    }

    poLogStorage = &m_oDefaultLogStorage;
    
    PLImp("OK, use default logstorage");

    return 0;
}

int PNode :: InitNetWork(const Options & oOptions, NetWork *& poNetWork)
{
    if (oOptions.poNetWork != nullptr)
    {
        poNetWork = oOptions.poNetWork;
        PLImp("OK, use user network");
        return 0;
    }

    int ret = m_oDefaultNetWork.Init(oOptions.oMyNode.GetIP(), oOptions.oMyNode.GetPort());
    if (ret != 0)
    {
        PLErr("init default network fail, listenip %s listenport %d ret %d",
                oOptions.oMyNode.GetIP().c_str(), oOptions.oMyNode.GetPort(), ret);
        return ret;
    }

    poNetWork = &m_oDefaultNetWork;
    
    PLImp("OK, use default network");

    return 0;
}

int PNode :: CheckOptions(const Options & oOptions)
{
    //init logger
    if (oOptions.pLogFunc != nullptr)
    {
        LOGGER->SetLogFunc(oOptions.pLogFunc);
    }
    else
    {
        LOGGER->InitLogger(oOptions.eLogLevel);
    }
    
    if (oOptions.poLogStorage == nullptr && oOptions.sLogStoragePath.size() == 0)
    {
        PLErr("no logpath and logstorage is null");
        return -2;
    }

    if (oOptions.iUDPMaxSize > 64 * 1024)
    {
        PLErr("udp max size %zu is too large", oOptions.iUDPMaxSize);
        return -2;
    }

    if (oOptions.iGroupCount > 300)
    {
        PLErr("group count %d is too large", oOptions.iGroupCount);
        return -2;
    }

    if (oOptions.iGroupCount < 0)
    {
        PLErr("group count %d is small than zero", oOptions.iGroupCount);
        return -2;
    }
    
    for (auto & oFollowerNodeInfo : oOptions.vecFollowerNodeInfoList)
    {
        if (oFollowerNodeInfo.oMyNode.GetNodeID() == oFollowerNodeInfo.oFollowNode.GetNodeID())
        {
            PLErr("self node ip %s port %d equal to follow node",
                    oFollowerNodeInfo.oMyNode.GetIP().c_str(), oFollowerNodeInfo.oMyNode.GetPort());
            return -2;
        }
    }

    for (auto & oGroupSMInfo : oOptions.vecGroupSMInfoList)
    {
        if (oGroupSMInfo.iGroupIdx >= oOptions.iGroupCount)
        {
            PLErr("SM GroupIdx %d large than GroupCount %d",
                    oGroupSMInfo.iGroupIdx, oOptions.iGroupCount);
            return -2;
        }
    }

    return 0;
}

void PNode :: InitStateMachine(const Options & oOptions)
{
    for (auto & oGroupSMInfo : oOptions.vecGroupSMInfoList)
    {
        for (auto & poSM : oGroupSMInfo.vecSMList)
        {
            AddStateMachine(oGroupSMInfo.iGroupIdx, poSM);
        }

        //check if need to run master.
        if (oGroupSMInfo.bIsUseMaster)
        {
            if (!m_vecGroupList[oGroupSMInfo.iGroupIdx]->GetConfig()->IsIMFollower())
            {
                m_vecMasterList[oGroupSMInfo.iGroupIdx]->RunMaster();
            }
            else
            {
                PLImp("I'm follower, not run master damon.");
            }
        }
    }
}

int PNode :: Init(const Options & oOptions, NetWork *& poNetWork)
{
    int ret = CheckOptions(oOptions);
    if (ret != 0)
    {
        PLErr("CheckOptions fail, ret %d", ret);
        return ret;
    }

    m_iMyNodeID = oOptions.oMyNode.GetNodeID();

    //step1 init logstorage
    LogStorage * poLogStorage = nullptr;
    ret = InitLogStorage(oOptions, poLogStorage);
    if (ret != 0)
    {
        return ret;
    }

    //step2 init network
    ret = InitNetWork(oOptions, poNetWork);
    if (ret != 0)
    {
        return ret;
    }

    //step3 build masterlist
    for (int iGroupIdx = 0; iGroupIdx < oOptions.iGroupCount; iGroupIdx++)
    {
        MasterDamon * poMaster = new MasterDamon(this, iGroupIdx, poLogStorage);
        
        assert(poMaster != nullptr);

        ret = poMaster->Init();
        if (ret != 0)
        {
            return ret;
        }

        m_vecMasterList.push_back(poMaster);
    }

    //step4 build grouplist
    for (int iGroupIdx = 0; iGroupIdx < oOptions.iGroupCount; iGroupIdx++)
    {
        Group * poGroup = new Group(poLogStorage, poNetWork, m_vecMasterList[iGroupIdx]->GetMasterSM(), iGroupIdx, oOptions);

        assert(poGroup != nullptr);

        m_vecGroupList.push_back(poGroup);
    }

    //step5 init statemachine
    InitStateMachine(oOptions);    

    //step6 init group
    for (auto & poGroup : m_vecGroupList)
    {
        int ret = poGroup->Init();
        if (ret != 0)
        {
            return ret;
        }
    }

    PLHead("OK");

    return 0;
}

bool PNode :: CheckGroupID(const int iGroupIdx)
{
    if (iGroupIdx < 0 || iGroupIdx >= (int)m_vecGroupList.size())
    {
        return false;
    }

    return true;
}

int PNode :: Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetCommitter()->NewValueGetID(sValue, llInstanceID);
}

int PNode :: Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, StateMachine * poSM)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetCommitter()->NewValueGetID(sValue, llInstanceID, poSM, nullptr);
}

int PNode :: Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, SMCtx * poSMCtx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetCommitter()->NewValueGetID(sValue, llInstanceID, nullptr, poSMCtx);
}

const uint64_t PNode :: GetNowInstanceID(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return (uint64_t)-1;
    }

    return m_vecGroupList[iGroupIdx]->GetInstance()->GetNowInstanceID();
}

int PNode :: OnReceiveMessage(const char * pcMessage, const int iMessageLen)
{
    if (pcMessage == nullptr || iMessageLen <= 0)
    {
        PLErr("Message size %d to small, not valid.", iMessageLen);
        return -2;
    }
    
    int iGroupIdx = -1;

    memcpy(&iGroupIdx, pcMessage, GROUPIDXLEN);

    if (!CheckGroupID(iGroupIdx))
    {
        PLErr("Message groupid %d wrong, groupsize %zu", iGroupIdx, m_vecGroupList.size());
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetInstance()->OnReceiveMessage(pcMessage, iMessageLen);
}

void PNode :: AddStateMachine(StateMachine * poSM)
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->AddStateMachine(poSM);
    }
}

void PNode :: AddStateMachine(const int iGroupIdx, StateMachine * poSM)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return;
    }
    
    m_vecGroupList[iGroupIdx]->AddStateMachine(poSM);
}

const nodeid_t PNode :: GetMyNodeID() const
{
    return m_iMyNodeID;
}

void PNode :: SetTimeoutMs(const int iTimeoutMs)
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCommitter()->SetTimeoutMs(iTimeoutMs);
    }
}

////////////////////////////////////////////////////////////////////////

void PNode :: SetHoldPaxosLogCount(const uint64_t llHoldCount)
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointCleaner()->SetHoldPaxosLogCount(llHoldCount);
    }
}

void PNode :: PauseCheckpointReplayer()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointReplayer()->Pause();
    }
}

void PNode :: ContinueCheckpointReplayer()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointReplayer()->Continue();
    }
}

void PNode :: PausePaxosLogCleaner()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointCleaner()->Pause();
    }
}

void PNode :: ContinuePaxosLogCleaner()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointCleaner()->Continue();
    }
}

///////////////////////////////////////////////////////

const NodeInfo PNode :: GetMaster(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return NodeInfo(nullnode);
    }

    return NodeInfo(m_vecMasterList[iGroupIdx]->GetMasterSM()->GetMaster());
}

const bool PNode :: IsIMMaster(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return false;
    }

    return m_vecMasterList[iGroupIdx]->GetMasterSM()->IsIMMaster();
}

int PNode :: SetMasterLease(const int iGroupIdx, const int iLeaseTimeMs)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    m_vecMasterList[iGroupIdx]->SetLeaseTime(iLeaseTimeMs);
    return 0;
}

int PNode :: DropMaster(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    m_vecMasterList[iGroupIdx]->DropMaster();
    return 0;
}
    
}

