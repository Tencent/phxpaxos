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

#include "phxpaxos/breakpoint.h"

namespace phxpaxos
{

Breakpoint * Breakpoint :: m_poBreakpoint = nullptr;

Breakpoint :: Breakpoint() 
{
}

void Breakpoint :: SetInstance(Breakpoint * poBreakpoint)
{
    m_poBreakpoint = poBreakpoint;
}

Breakpoint * Breakpoint :: Instance()
{
    if (m_poBreakpoint != nullptr)
    {
        return m_poBreakpoint;
    }
    
    static Breakpoint oBreakpoint;
    return &oBreakpoint;
}
    
ProposerBP * Breakpoint :: GetProposerBP()
{
    return &m_oProposerBP;
}

AcceptorBP * Breakpoint :: GetAcceptorBP()
{
    return &m_oAcceptorBP;
}

LearnerBP * Breakpoint :: GetLearnerBP()
{
    return &m_oLearnerBP;
}

InstanceBP * Breakpoint :: GetInstanceBP()
{
    return &m_oInstanceBP;
}

CommiterBP * Breakpoint :: GetCommiterBP()
{
    return &m_oCommiterBP;
}

IOLoopBP * Breakpoint :: GetIOLoopBP()
{
    return &m_oIOLoopBP;
}

NetworkBP * Breakpoint :: GetNetworkBP()
{
    return &m_oNetworkBP;
}

LogStorageBP * Breakpoint :: GetLogStorageBP()
{
    return &m_oLogStorageBP;
}

AlgorithmBaseBP * Breakpoint :: GetAlgorithmBaseBP()
{
    return &m_oAlgorithmBaseBP;
}

CheckpointBP * Breakpoint :: GetCheckpointBP()
{
    return &m_oCheckpointBP;
}

MasterBP * Breakpoint :: GetMasterBP()
{
    return &m_oMasterBP;
}
    
}


