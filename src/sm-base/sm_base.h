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
#include <vector>
#include "phxpaxos/sm.h"

namespace phxpaxos
{
    
class SMFac
{
public:
    SMFac();
    ~SMFac();
    
    bool DoExecute(StateMachine * poSM, const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, SMCtx * poSMCtx);

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, SMCtx * poSMCtx);

    bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, const std::string & sPaxosValue);

    void PackPaxosValue(std::string & sPaxosValue, const int iSMID = 0);

    void AddSM(StateMachine * poSM);

public:
    const uint64_t GetCheckpointInstanceID(const int iGroupIdx) const;

    std::vector<StateMachine *> GetSMList();

public:
    

private:
    std::vector<StateMachine *> m_vecSMList;
};
    
}
