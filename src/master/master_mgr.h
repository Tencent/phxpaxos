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

#include "commdef.h"
#include "utils_include.h"
#include "phxpaxos/node.h"
#include "master_sm.h"

namespace phxpaxos 
{

class MasterMgr : public Thread
{
public:
    MasterMgr(const Node * poPaxosNode, 
        const int iGroupIdx, 
        const LogStorage * poLogStorage,
        MasterChangeCallback pMasterChangeCallback);
    ~MasterMgr();

    void RunMaster();
    
    void StopMaster();

    int Init();

    void run();

    void SetLeaseTime(const int iLeaseTimeMs);

    void TryBeMaster(const int iLeaseTime);

    void DropMaster();

public:
    MasterStateMachine * GetMasterSM();

private:
    Node * m_poPaxosNode;

    MasterStateMachine m_oDefaultMasterSM;

private:
    int m_iLeaseTime;

    bool m_bIsEnd;
    bool m_bIsStarted;

    int m_iMyGroupIdx;

    bool m_bNeedDropMaster;
};
    
}
