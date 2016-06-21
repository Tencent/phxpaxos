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
#include "kv.h"
#include <limits>
#include "def.h"

namespace phxkv
{

class PhxKVSMCtx
{
public:
    int iExecuteRet;
    std::string sReadValue;
    uint64_t llReadVersion;

    PhxKVSMCtx()
    {
        iExecuteRet = -1;
        llReadVersion = 0;
    }
};

////////////////////////////////////////////////

class PhxKVSM : public phxpaxos::StateMachine
{
public:
    PhxKVSM(const std::string & sDBPath);
    ~PhxKVSM();

    const bool Init();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, phxpaxos::SMCtx * poSMCtx);

    const int SMID() const {return 1;}

public:
    //no use
    bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue) {return true;}

    //have checkpoint.
    const uint64_t GetCheckpointInstanceID(const int iGroupIdx) const { return m_llCheckpointInstanceID;}

public:
    //have checkpoint, but not impl auto copy checkpoint to other node, so return fail.
    int LockCheckpointState() { return -1; }
    
    int GetCheckpointState(const int iGroupIdx, std::string & sDirPath, 
            std::vector<std::string> & vecFileList) { return -1; }

    void UnLockCheckpointState() { }
    
    int LoadCheckpointState(const int iGroupIdx, const std::string & sCheckpointTmpFileDirPath,
            const std::vector<std::string> & vecFileList, const uint64_t llCheckpointInstanceID) { return -1; }

public:
    static bool MakeOpValue(
            const std::string & sKey, 
            const std::string & sValue, 
            const uint64_t llVersion, 
            const KVOperatorType iOp,
            std::string & sPaxosValue);

    static bool MakeGetOpValue(
            const std::string & sKey,
            std::string & sPaxosValue);

    static bool MakeSetOpValue(
            const std::string & sKey, 
            const std::string & sValue, 
            const uint64_t llVersion, 
            std::string & sPaxosValue);

    static bool MakeDelOpValue(
            const std::string & sKey, 
            const uint64_t llVersion, 
            std::string & sPaxosValue);

    KVClient * GetKVClient();

    int SyncCheckpointInstanceID(const uint64_t llInstanceID);

private:
    std::string m_sDBPath;
    KVClient m_oKVClient;

    uint64_t m_llCheckpointInstanceID;
    int m_iSkipSyncCheckpointTimes;
};
    
}
