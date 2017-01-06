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

#include "phxpaxos/sm.h"
#include "comm_include.h"

namespace phxpaxos
{

SMCtx :: SMCtx(const int iSMID, void * pCtx) : m_iSMID(iSMID), m_pCtx(pCtx)
{
}

SMCtx :: SMCtx() : m_iSMID(0), m_pCtx(nullptr)
{
}

bool StateMachine :: ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sPaxosValue) 
{ 
    return true; 
}

const uint64_t StateMachine :: GetCheckpointInstanceID(const int iGroupIdx) const 
{ 
    return phxpaxos::NoCheckpoint;
}

//default no checkpoint 
int StateMachine :: GetCheckpointState(const int iGroupIdx, std::string & sDirPath, 
        std::vector<std::string> & vecFileList) 
{ 
    PLErr("func not impl, return -1");
    return -1; 
}    

int StateMachine :: LoadCheckpointState(const int iGroupIdx, const std::string & sCheckpointTmpFileDirPath,
        const std::vector<std::string> & vecFileList, const uint64_t llCheckpointInstanceID) 
{ 
    PLErr("func not impl, return -1");
    return -1;
}

int StateMachine :: LockCheckpointState() 
{ 
    PLErr("func not impl, return -1");
    return -1; 
}

void StateMachine :: UnLockCheckpointState() 
{ 
}

void StateMachine :: BeforePropose(const int iGroupIdx, std::string & sValue)
{
}

const bool StateMachine :: NeedCallBeforePropose()
{
    return false;
}

}


