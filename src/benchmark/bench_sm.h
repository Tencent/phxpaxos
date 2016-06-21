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

#include "phxpaxos/sm.h"
#include "phxpaxos/options.h"
#include <stdio.h>
#include <unistd.h>

namespace bench
{

class BenchSM : public phxpaxos::StateMachine
{
public:
    BenchSM(const phxpaxos::nodeid_t llMyNodeID, const int iGroupIdx);

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, phxpaxos::SMCtx * poSMCtx);

    const int SMID() const { return 1; }
    
    //no checkpoint 
    bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue) { return true; }

    const uint64_t GetCheckpointInstanceID(const int iGroupIdx) const { return phxpaxos::NoCheckpoint; }

    const int GetGroupIdx() const;

public:
    //no checkpoint 
    int GetCheckpointState(const int iGroupIdx, std::string & sDirPath, 
            std::vector<std::string> & vecFileList) { return 0; }    
    
    //no checkpoint 
    int LoadCheckpointState(const int iGroupIdx, const std::string & sCheckpointTmpFileDirPath,
            const std::vector<std::string> & vecFileList, const uint64_t llCheckpointInstanceID) { return 0; }

    //no checkpoint 
    int LockCheckpointState() { return 0; }

    //no checkpoint 
    void UnLockCheckpointState() { }

private:
    phxpaxos::nodeid_t m_llMyNodeID;
    int m_iGroupIdx;
};
    
}
