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

#include "master_mgr.h"
#include "comm_include.h"
#include "commdef.h"

namespace phxpaxos 
{

MasterMgr :: MasterMgr(
    const Node * poPaxosNode, 
    const int iGroupIdx, 
    const LogStorage * poLogStorage,
    MasterChangeCallback pMasterChangeCallback) 
    : m_oDefaultMasterSM(poLogStorage, poPaxosNode->GetMyNodeID(), iGroupIdx, pMasterChangeCallback) 
{
    m_iLeaseTime = 10000;

    m_poPaxosNode = (Node *)poPaxosNode;
    m_iMyGroupIdx = iGroupIdx;
    
    m_bIsEnd = false;
    m_bIsStarted = false;
    
    m_bNeedDropMaster = false;
}

MasterMgr :: ~MasterMgr()
{
}

int MasterMgr :: Init()
{
    return m_oDefaultMasterSM.Init();
}

void MasterMgr :: SetLeaseTime(const int iLeaseTimeMs)
{
    if (iLeaseTimeMs < 1000)
    {
        return;
    }

    m_iLeaseTime = iLeaseTimeMs;
}

void MasterMgr :: DropMaster()
{
    m_bNeedDropMaster = true;
}

void MasterMgr :: StopMaster()
{
    if (m_bIsStarted)
    {
        m_bIsEnd = true;
        join();
    }
}

void MasterMgr :: RunMaster()
{
    start();
}

void MasterMgr :: run()
{
    m_bIsStarted = true;

    while(true)
    {
        if (m_bIsEnd)
        {
            return;
        }
        
        int iLeaseTime = m_iLeaseTime;

        uint64_t llBeginTime = Time::GetSteadyClockMS();
        
        TryBeMaster(iLeaseTime);

        int iContinueLeaseTimeout = (iLeaseTime - 100) / 4;
        iContinueLeaseTimeout = iContinueLeaseTimeout / 2 + OtherUtils::FastRand() % iContinueLeaseTimeout;

        if (m_bNeedDropMaster)
        {
            BP->GetMasterBP()->DropMaster();
            m_bNeedDropMaster = false;
            iContinueLeaseTimeout = iLeaseTime * 2;
            PLG1Imp("Need drop master, this round wait time %dms", iContinueLeaseTimeout);
        }
        
        uint64_t llEndTime = Time::GetSteadyClockMS();
        int iRunTime = llEndTime > llBeginTime ? llEndTime - llBeginTime : 0;
        int iNeedSleepTime = iContinueLeaseTimeout > iRunTime ? iContinueLeaseTimeout - iRunTime : 0;

        PLG1Imp("TryBeMaster, sleep time %dms", iNeedSleepTime);
        Time::MsSleep(iNeedSleepTime);
    }
}

void MasterMgr :: TryBeMaster(const int iLeaseTime)
{
    nodeid_t iMasterNodeID = nullnode;
    uint64_t llMasterVersion = 0;

    //step 1 check exist master and get version
    m_oDefaultMasterSM.SafeGetMaster(iMasterNodeID, llMasterVersion);

    if (iMasterNodeID != nullnode && (iMasterNodeID != m_poPaxosNode->GetMyNodeID()))
    {
        PLG1Imp("Ohter as master, can't try be master, masterid %lu myid %lu", 
                iMasterNodeID, m_poPaxosNode->GetMyNodeID());
        return;
    }

    BP->GetMasterBP()->TryBeMaster();

    //step 2 try be master
    std::string sPaxosValue;
    if (!MasterStateMachine::MakeOpValue(
                m_poPaxosNode->GetMyNodeID(),
                llMasterVersion,
                iLeaseTime,
                MasterOperatorType_Complete,
                sPaxosValue))
    {
        PLG1Err("Make paxos value fail");
        return;
    }

    const int iMasterLeaseTimeout = iLeaseTime - 100;
    
    uint64_t llAbsMasterTimeout = Time::GetSteadyClockMS() + iMasterLeaseTimeout; 
    uint64_t llCommitInstanceID = 0;

    SMCtx oCtx;
    oCtx.m_iSMID = MASTER_V_SMID;
    oCtx.m_pCtx = (void *)&llAbsMasterTimeout;

    int ret = m_poPaxosNode->Propose(m_iMyGroupIdx, sPaxosValue, llCommitInstanceID, &oCtx);
    if (ret != 0)
    {
        BP->GetMasterBP()->TryBeMasterProposeFail();
    }
}

MasterStateMachine * MasterMgr :: GetMasterSM()
{
    return &m_oDefaultMasterSM;
}

    
}


