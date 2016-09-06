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

#include "config.h"
#include <math.h>
#include "inttypes.h"
#include "comm_include.h"

namespace phxpaxos
{

Config :: Config(
        const LogStorage * poLogStorage,
        const bool bLogSync,
        const int iSyncInterval,
        const bool bUseMembership,
        const NodeInfo & oMyNode, 
        const NodeInfoList & vecNodeInfoList,
        const FollowerNodeInfoList & vecFollowerNodeInfoList,
        const int iMyGroupIdx,
        const int iGroupCount,
        MembershipChangeCallback pMembershipChangeCallback)
    : m_bLogSync(bLogSync), 
    m_iSyncInterval(iSyncInterval),
    m_bUseMembership(bUseMembership),
    m_iMyNodeID(oMyNode.GetNodeID()), 
    m_iNodeCount(vecNodeInfoList.size()), 
    m_iMyGroupIdx(iMyGroupIdx),
    m_iGroupCount(iGroupCount),
    m_oSystemVSM(iMyGroupIdx, oMyNode.GetNodeID(), poLogStorage, pMembershipChangeCallback),
    m_poMasterSM(nullptr)
{
    m_vecNodeInfoList = vecNodeInfoList;

    m_bIsIMFollower = false;
    m_iFollowToNodeID = nullnode;

    for (auto & oFollowerNodeInfo : vecFollowerNodeInfoList)
    {
        if (oFollowerNodeInfo.oMyNode.GetNodeID() == oMyNode.GetNodeID())
        {
            PLG1Head("I'm follower, ip %s port %d nodeid %lu",
                    oMyNode.GetIP().c_str(), oMyNode.GetPort(), oMyNode.GetNodeID());
            m_bIsIMFollower = true;
            m_iFollowToNodeID = oFollowerNodeInfo.oFollowNode.GetNodeID();

            InsideOptions::Instance()->SetAsFollower();
        }
    }
}

Config :: ~Config()
{
}

int Config :: Init()
{
    int ret = m_oSystemVSM.Init();
    if (ret != 0)
    {
        PLG1Err("fail, ret %d", ret);
        return ret;
    }

    m_oSystemVSM.AddNodeIDList(m_vecNodeInfoList);

    PLG1Head("OK");
    return 0;
}

const bool Config :: CheckConfig()
{
    if (!m_oSystemVSM.IsIMInMembership())
    {
        PLG1Err("my node %lu is not in membership", m_iMyNodeID);
        return false;
    }

    return true;
}

const uint64_t Config :: GetGid() const
{
    return m_oSystemVSM.GetGid();
}

const nodeid_t Config :: GetMyNodeID() const
{
    return m_iMyNodeID;
}

const int Config :: GetNodeCount() const
{
    return m_oSystemVSM.GetNodeCount();
}

const int Config :: GetMyGroupIdx() const
{
    return m_iMyGroupIdx;
}

const int Config :: GetGroupCount() const
{
    return m_iGroupCount;
}

const int Config :: GetMajorityCount() const
{
    return m_oSystemVSM.GetMajorityCount();
}

const bool Config :: GetIsUseMembership() const
{
    return m_bUseMembership;
}

////////////////////////////////////////////////////////////

const uint64_t Config :: GetAskforLearnTimeoutMs() const
{
    return 2000;
}

const int Config :: GetPrepareTimeoutMs() const
{
    return 3000;
}

const int Config :: GetAcceptTimeoutMs() const
{
    return 3000;
}

///////////////////////////////////////////////////////////

const bool Config :: IsValidNodeID(const nodeid_t iNodeID)
{
    return m_oSystemVSM.IsValidNodeID(iNodeID);
}

const bool Config :: IsIMFollower() const
{
    return m_bIsIMFollower;
}

const nodeid_t Config :: GetFollowToNodeID() const
{
    return m_iFollowToNodeID;
}

///////////////////////////////////////////////////////

SystemVSM * Config :: GetSystemVSM()
{
    return &m_oSystemVSM;
}

///////////////////////////////////////////////////////

void Config :: SetMasterSM(InsideSM * poMasterSM)
{
    m_poMasterSM = poMasterSM;
}

InsideSM * Config :: GetMasterSM()
{
    return m_poMasterSM;
}

///////////////////////////////////////////////////////

#define TmpNodeTimeout 60000

void Config :: AddTmpNodeOnlyForLearn(const nodeid_t iTmpNodeID)
{
    const std::set<nodeid_t> & setNodeID = m_oSystemVSM.GetMembershipMap();
    if (setNodeID.find(iTmpNodeID) != end(setNodeID))
    {
        return;
    }

    m_mapTmpNodeOnlyForLearn[iTmpNodeID] = Time::GetSteadyClockMS() + TmpNodeTimeout;
}

const std::map<nodeid_t, uint64_t> & Config :: GetTmpNodeMap()
{
    uint64_t llNowTime = Time::GetSteadyClockMS();

    for (auto it = m_mapTmpNodeOnlyForLearn.begin(); it != end(m_mapTmpNodeOnlyForLearn);)
    {
        if (it->second < llNowTime)
        {
            PLErr("tmpnode %lu timeout, nowtimems %lu tmpnode last add time %lu",
                    it->first, llNowTime, it->second);
            it = m_mapTmpNodeOnlyForLearn.erase(it);
        }
        else
        {
            it++;
        }
    }

    return m_mapTmpNodeOnlyForLearn;
}

void Config :: AddFollowerNode(const nodeid_t iMyFollowerNodeID)
{
    static int iFollowerTimeout = ASKFORLEARN_NOOP_INTERVAL * 3;
    m_mapMyFollower[iMyFollowerNodeID] = Time::GetSteadyClockMS() + iFollowerTimeout;
}

const std::map<nodeid_t, uint64_t> & Config :: GetMyFollowerMap()
{
    uint64_t llNowTime = Time::GetSteadyClockMS();

    for (auto it = m_mapMyFollower.begin(); it != end(m_mapMyFollower);)
    {
        if (it->second < llNowTime)
        {
            PLErr("follower %lu timeout, nowtimems %lu tmpnode last add time %lu",
                    it->first, llNowTime, it->second);
            it = m_mapMyFollower.erase(it);
        }
        else
        {
            it++;
        }
    }

    return m_mapMyFollower;
}

const size_t Config :: GetMyFollowerCount()
{
    return m_mapMyFollower.size();
}

const bool Config :: LogSync() const
{
    return m_bLogSync;
}

void Config :: SetLogSync(const bool LogSync) 
{
    m_bLogSync = LogSync;
}

const int Config :: SyncInterval() const 
{
    return m_iSyncInterval;
}

}


