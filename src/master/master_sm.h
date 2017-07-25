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

#include <mutex>
#include "phxpaxos/sm.h"
#include "commdef.h"
#include "phxpaxos/def.h"
#include "config_include.h"
#include "master_sm.pb.h"
#include "master_sm.h"
#include "master_variables_store.h"
#include "utils_include.h"

namespace phxpaxos 
{

enum MasterOperatorType
{
    MasterOperatorType_Complete = 1,
};

class MasterStateMachine : public InsideSM 
{
public:
    MasterStateMachine(
        const LogStorage * poLogStorage, 
        const nodeid_t iMyNodeID, 
        const int iGroupIdx,
        MasterChangeCallback pMasterChangeCallback);
    ~MasterStateMachine();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, const std::string & sValue, SMCtx * poSMCtx);

    const int SMID() const {return MASTER_V_SMID;}

    bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue)
    {
        return true;
    }

    const uint64_t GetCheckpointInstanceID(const int iGroupIdx) const
    {
        return m_llMasterVersion;
    }

    void BeforePropose(const int iGroupIdx, std::string & sValue);

    const bool NeedCallBeforePropose();

public:
    int GetCheckpointState(const int iGroupIdx, std::string & sDirPath, 
            std::vector<std::string> & vecFileList)
    {
        return 0;
    }    
    
    int LoadCheckpointState(const int iGroupIdx, const std::string & sCheckpointTmpFileDirPath,
            const std::vector<std::string> & vecFileList, const uint64_t llCheckpointInstanceID)
    {
        return 0;
    }

    int LockCheckpointState()
    {
        return 0;
    }

    void UnLockCheckpointState()
    {
    }

public:
    int Init();

    int LearnMaster(
            const uint64_t llInstanceID,
            const MasterOperator & oMasterOper, 
            const uint64_t llAbsMasterTimeout = 0);

    const nodeid_t GetMaster() const;

    const nodeid_t GetMasterWithVersion(uint64_t & llVersion);

    const bool IsIMMaster() const;

public:
    int UpdateMasterToStore(const nodeid_t llMasterNodeID, const uint64_t llVersion, const uint32_t iLeaseTime);

    void SafeGetMaster(nodeid_t & iMasterNodeID, uint64_t & llMasterVersion);

public:
    static bool MakeOpValue(
            const nodeid_t iNodeID, 
            const uint64_t llVersion,
            const int iTimeout,
            const MasterOperatorType iOp,    
            std::string & sPaxosValue);

public:
    int GetCheckpointBuffer(std::string & sCPBuffer);

    int UpdateByCheckpoint(const std::string & sCPBuffer, bool & bChange);

private:
    int m_iMyGroupIdx;
    nodeid_t m_iMyNodeID;

private:
    MasterVariablesStore m_oMVStore;
    
    nodeid_t m_iMasterNodeID;
    uint64_t m_llMasterVersion;
    int m_iLeaseTime;
    uint64_t m_llAbsExpireTime;

    std::mutex m_oMutex;

    MasterChangeCallback m_pMasterChangeCallback;
};
    
}
