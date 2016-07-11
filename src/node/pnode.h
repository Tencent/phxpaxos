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

#include "phxpaxos/node.h"
#include "phxpaxos/options.h"
#include <vector>
#include "db.h"
#include "dfnetwork.h"
#include "group.h"
#include "master_damon.h"

namespace phxpaxos
{

class PNode : public Node
{
public:
    PNode();

    ~PNode();

    int Init(const Options & oOptions, NetWork *& poNetWork);

public:
    int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID);

    int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, StateMachine * poSM);

    int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, SMCtx * poSMCtx);

    const uint64_t GetNowInstanceID(const int iGroupIdx);

public:
    void AddStateMachine(StateMachine * poSM);
    
    void AddStateMachine(const int iGroupIdx, StateMachine * poSM);
    
    int OnReceiveMessage(const char * pcMessage, const int iMessageLen);

    const nodeid_t GetMyNodeID() const;

    void SetTimeoutMs(const int iTimeoutMs);

public:
    void SetHoldPaxosLogCount(const uint64_t llHoldCount);

    void PauseCheckpointReplayer();

    void ContinueCheckpointReplayer();

    void PausePaxosLogCleaner();

    void ContinuePaxosLogCleaner();
    
public:
    //membership
    int AddMember(const int iGroupIdx, const NodeInfo & oNode);

    int RemoveMember(const int iGroupIdx, const NodeInfo & oNode);

    int ChangeMember(const int iGroupIdx, const NodeInfo & oFromNode, const NodeInfo & oToNode);
    
    int ShowMembership(const int iGroupIdx, NodeInfoList & vecNodeInfoList);

public:
    //master
    
    const NodeInfo GetMaster(const int iGroupIdx);

    const bool IsIMMaster(const int iGroupIdx);

    int SetMasterLease(const int iGroupIdx, const int iLeaseTimeMs);

    int DropMaster(const int iGroupIdx);

public:
    void SetMaxHoldThreads(const int iGroupIdx, const int iMaxHoldThreads);

    void SetProposeWaitTimeThresholdMS(const int iGroupIdx, const int iWaitTimeThresholdMS);

private:
    int CheckOptions(const Options & oOptions);

    int InitLogStorage(const Options & oOptions, LogStorage *& poLogStorage);

    int InitNetWork(const Options & oOptions, NetWork *& poNetWork);

    int InitMaster(const Options & oOptions);

    void InitStateMachine(const Options & oOptions);

    bool CheckGroupID(const int iGroupIdx);

    int ProposalMembership(
            SystemVSM * poSystemVSM,
            const int iGroupIdx, 
            const NodeInfoList & vecNodeInfoList, 
            const uint64_t llVersion);

private:
    std::vector<Group *> m_vecGroupList;

private:
    std::vector<MasterDamon *> m_vecMasterList;

private:
    MultiDatabase m_oDefaultLogStorage;
    DFNetWork m_oDefaultNetWork;

    nodeid_t m_iMyNodeID;
};
    
}
