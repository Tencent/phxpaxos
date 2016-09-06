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

#include <vector>
#include "commdef.h"
#include "system_v_sm.h"

namespace phxpaxos
{

class Config
{
public:
    Config(
        const LogStorage * poLogStorage,
        const bool bLogSync,
        const int iSyncInterval,
        const bool bUseMembership,
        const NodeInfo & oMyNode,
        const NodeInfoList & vecNodeInfoList,
        const FollowerNodeInfoList & vecFollowerNodeInfoList,
        const int iMyGroupIdx,
        const int iGroupCount,
        MembershipChangeCallback pMembershipChangeCallback);

    ~Config();

    int Init();

    const bool CheckConfig();

public:
    SystemVSM * GetSystemVSM();

public:
    const uint64_t GetGid() const;

    const nodeid_t GetMyNodeID() const;
    
    const int GetNodeCount() const;

    const int GetMyGroupIdx() const;

    const int GetGroupCount() const;

    const int GetMajorityCount() const;

    const bool GetIsUseMembership() const;

public:
    const int GetPrepareTimeoutMs() const;

    const int GetAcceptTimeoutMs() const;

    const uint64_t GetAskforLearnTimeoutMs() const;

public:
    const bool IsValidNodeID(const nodeid_t iNodeID);

    const bool IsIMFollower() const;

    const nodeid_t GetFollowToNodeID() const;

    const bool LogSync() const;

    const int SyncInterval() const;

    void SetLogSync(const bool bLogSync);

public:
    void SetMasterSM(InsideSM * poMasterSM);

    InsideSM * GetMasterSM();

public:
    void AddTmpNodeOnlyForLearn(const nodeid_t iTmpNodeID);

    //this function only for communicate.
    const std::map<nodeid_t, uint64_t> & GetTmpNodeMap();

    void AddFollowerNode(const nodeid_t iMyFollowerNodeID);

    //this function only for communicate.
    const std::map<nodeid_t, uint64_t> & GetMyFollowerMap();

    const size_t GetMyFollowerCount();

private:
    bool m_bLogSync;
    int m_iSyncInterval;
    bool m_bUseMembership;

    nodeid_t m_iMyNodeID;
    int m_iNodeCount;
    int m_iMyGroupIdx;
    int m_iGroupCount;

    NodeInfoList m_vecNodeInfoList;

    bool m_bIsIMFollower;
    nodeid_t m_iFollowToNodeID;

    SystemVSM m_oSystemVSM;
    InsideSM * m_poMasterSM;

    std::map<nodeid_t, uint64_t> m_mapTmpNodeOnlyForLearn;
    std::map<nodeid_t, uint64_t> m_mapMyFollower;
};

}
