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

#include "master_sm.h"
#include "master_sm.pb.h"
#include <math.h>
#include "commdef.h"
#include "comm_include.h"

namespace phxpaxos 
{

MasterStateMachine :: MasterStateMachine(
    const LogStorage * poLogStorage, 
    const nodeid_t iMyNodeID, 
    const int iGroupIdx,
    MasterChangeCallback pMasterChangeCallback)
    : m_oMVStore(poLogStorage), m_pMasterChangeCallback(pMasterChangeCallback)
{
    m_iMyGroupIdx = iGroupIdx;
    m_iMyNodeID = iMyNodeID;

    m_iMasterNodeID = nullnode;
    m_llMasterVersion = (uint64_t)-1;
    m_iLeaseTime = 0;
    m_llAbsExpireTime = 0;

}

MasterStateMachine :: ~MasterStateMachine()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

int MasterStateMachine :: Init()
{
    MasterVariables oVariables;
    int ret = m_oMVStore.Read(m_iMyGroupIdx, oVariables);
    if (ret != 0 && ret != 1)
    {
        PLG1Err("Master variables read from store fail, ret %d", ret);
        return -1;
    }

    if (ret == 1)
    {
        PLG1Imp("no master variables exist");
    }
    else
    {
        m_llMasterVersion = oVariables.version();

        if (oVariables.masternodeid() == m_iMyNodeID)
        {
            m_iMasterNodeID = nullnode;
            m_llAbsExpireTime = 0;
        }
        else
        {
            m_iMasterNodeID = oVariables.masternodeid();
            m_llAbsExpireTime = Time::GetSteadyClockMS() + oVariables.leasetime();
        }
    }
    
    PLG1Head("OK, master nodeid %lu version %lu expiretime %u", 
            m_iMasterNodeID, m_llMasterVersion, m_llAbsExpireTime);
    
    return 0;
}

int MasterStateMachine :: UpdateMasterToStore(const nodeid_t llMasterNodeID, const uint64_t llVersion, const uint32_t iLeaseTime)
{
    MasterVariables oVariables;
    oVariables.set_masternodeid(llMasterNodeID);
    oVariables.set_version(llVersion);
    oVariables.set_leasetime(iLeaseTime);

    WriteOptions oWriteOptions;
    oWriteOptions.bSync = true;
    
    return m_oMVStore.Write(oWriteOptions, m_iMyGroupIdx, oVariables);
}

int MasterStateMachine :: LearnMaster(
        const uint64_t llInstanceID, 
        const MasterOperator & oMasterOper, 
        const uint64_t llAbsMasterTimeout)
{
    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    PLG1Debug("my last version %lu other last version %lu this version %lu instanceid %lu",
            m_llMasterVersion, oMasterOper.lastversion(), oMasterOper.version(), llInstanceID);

    if (oMasterOper.lastversion() != 0
            && llInstanceID > m_llMasterVersion
            && oMasterOper.lastversion() != m_llMasterVersion)
    {
        BP->GetMasterBP()->MasterSMInconsistent();
        PLG1Err("other last version %lu not same to my last version %lu, instanceid %lu",
                oMasterOper.lastversion(), m_llMasterVersion, llInstanceID);

        PLG1Err("try to fix, set my master version %lu as other last version %lu, instanceid %lu",
                m_llMasterVersion, oMasterOper.lastversion(), llInstanceID);
        m_llMasterVersion = oMasterOper.lastversion();
    }

    if (oMasterOper.version() != m_llMasterVersion)
    {
        PLG1Debug("version conflit, op version %lu now master version %lu",
                oMasterOper.version(), m_llMasterVersion);
        return 0;
    }

    int ret = UpdateMasterToStore(oMasterOper.nodeid(), llInstanceID, oMasterOper.timeout());
    if (ret != 0)
    {
        PLG1Err("UpdateMasterToStore fail, ret %d", ret);
        return -1;
    }

    bool bMasterChange = false;
    if (m_iMasterNodeID != oMasterOper.nodeid())
    {
        bMasterChange = true;
    }

    m_iMasterNodeID = oMasterOper.nodeid();
    if (m_iMasterNodeID == m_iMyNodeID)
    {
        //self be master
        //use local abstimeout
        m_llAbsExpireTime = llAbsMasterTimeout;

        BP->GetMasterBP()->SuccessBeMaster();
        PLG1Head("Be master success, absexpiretime %lu", m_llAbsExpireTime);
    }
    else
    {
        //other be master
        //use new start timeout
        m_llAbsExpireTime = Time::GetSteadyClockMS() + oMasterOper.timeout();

        BP->GetMasterBP()->OtherBeMaster();
        PLG1Head("Ohter be master, absexpiretime %lu", m_llAbsExpireTime);
    }

    m_iLeaseTime = oMasterOper.timeout();
    m_llMasterVersion = llInstanceID;

    if (bMasterChange)
    {
        if (m_pMasterChangeCallback != nullptr)
        {
            m_pMasterChangeCallback(m_iMyGroupIdx, NodeInfo(m_iMasterNodeID), m_llMasterVersion);
        }
    }

    PLG1Imp("OK, masternodeid %lu version %lu abstimeout %lu",
            m_iMasterNodeID, m_llMasterVersion, m_llAbsExpireTime);

    return 0;
}

void MasterStateMachine :: SafeGetMaster(nodeid_t & iMasterNodeID, uint64_t & llMasterVersion)
{
    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    if (Time::GetSteadyClockMS() >= m_llAbsExpireTime)
    {
        iMasterNodeID = nullnode;
    }
    else
    {
        iMasterNodeID = m_iMasterNodeID;
    }

    llMasterVersion = m_llMasterVersion;
}

const nodeid_t MasterStateMachine :: GetMaster() const
{
    if (Time::GetSteadyClockMS() >= m_llAbsExpireTime)
    {
        return nullnode;
    }

    return m_iMasterNodeID;
}

const nodeid_t MasterStateMachine :: GetMasterWithVersion(uint64_t & llVersion) 
{
    nodeid_t iMasterNodeID = nullnode;
    SafeGetMaster(iMasterNodeID, llVersion);
    return iMasterNodeID;
}

const bool MasterStateMachine :: IsIMMaster() const
{
    nodeid_t iMasterNodeID = GetMaster();
    return iMasterNodeID == m_iMyNodeID;
}

////////////////////////////////////////////////////////////////////////////////////////////

bool MasterStateMachine :: Execute(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sValue, SMCtx * poSMCtx)
{
    MasterOperator oMasterOper;
    bool bSucc = oMasterOper.ParseFromArray(sValue.data(), sValue.size());
    if (!bSucc)
    {
        PLG1Err("oMasterOper data wrong");
        return false;
    }

    if (oMasterOper.operator_() == MasterOperatorType_Complete)
    {
        uint64_t * pAbsMasterTimeout = nullptr;
        if (poSMCtx != nullptr && poSMCtx->m_pCtx != nullptr)
        {
            pAbsMasterTimeout = (uint64_t *)poSMCtx->m_pCtx;
        }

        uint64_t llAbsMasterTimeout = pAbsMasterTimeout != nullptr ? *pAbsMasterTimeout : 0;

        PLG1Imp("absmaster timeout %lu", llAbsMasterTimeout);

        int ret = LearnMaster(llInstanceID, oMasterOper, llAbsMasterTimeout);
        if (ret != 0)
        {
            return false;
        }
    }
    else
    {
        PLG1Err("unknown op %u", oMasterOper.operator_());
        //wrong op, just skip, so return true;
        return true;
    }

    return true;
}

////////////////////////////////////////////////////

bool MasterStateMachine :: MakeOpValue(
        const nodeid_t iNodeID,
        const uint64_t llVersion,
        const int iTimeout,
        const MasterOperatorType iOp,
        std::string & sPaxosValue)
{
    MasterOperator oMasterOper;
    oMasterOper.set_nodeid(iNodeID);
    oMasterOper.set_version(llVersion);
    oMasterOper.set_timeout(iTimeout);
    oMasterOper.set_operator_(iOp);
    oMasterOper.set_sid(OtherUtils::FastRand());

    return oMasterOper.SerializeToString(&sPaxosValue);
}

////////////////////////////////////////////////////////////

int MasterStateMachine :: GetCheckpointBuffer(std::string & sCPBuffer)
{
    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    if (m_llMasterVersion == (uint64_t)-1)
    {
        return 0;
    }
    
    MasterVariables oVariables;
    oVariables.set_masternodeid(m_iMasterNodeID);
    oVariables.set_version(m_llMasterVersion);
    oVariables.set_leasetime(m_iLeaseTime);
    
    bool sSucc = oVariables.SerializeToString(&sCPBuffer);
    if (!sSucc)
    {
        PLG1Err("Variables.Serialize fail");
        return -1;
    }

    return 0;
}

int MasterStateMachine :: UpdateByCheckpoint(const std::string & sCPBuffer, bool & bChange)
{
    if (sCPBuffer.size() == 0)
    {
        return 0;
    }

    MasterVariables oVariables;
    bool bSucc = oVariables.ParseFromArray(sCPBuffer.data(), sCPBuffer.size());
    if (!bSucc)
    {
        PLG1Err("Variables.ParseFromArray fail, bufferlen %zu", sCPBuffer.size());
        return -1;
    }

    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    if (oVariables.version() <= m_llMasterVersion
            && m_llMasterVersion != (uint64_t)-1)
    {
        PLG1Imp("lag checkpoint, no need update, cp.version %lu now.version %lu",
                oVariables.version(), m_llMasterVersion);
        return 0;
    }


    int ret = UpdateMasterToStore(oVariables.masternodeid(), oVariables.version(), oVariables.leasetime());
    if (ret != 0)
    {
        return -1;
    }

    PLG1Head("ok, cp.version %lu cp.masternodeid %lu old.version %lu old.masternodeid %lu", 
            oVariables.version(), oVariables.masternodeid(),
            m_llMasterVersion, m_iMasterNodeID);

    bool bMasterChange = false;
    m_llMasterVersion = oVariables.version();

    if (oVariables.masternodeid() == m_iMyNodeID)
    {
        m_iMasterNodeID = nullnode;
        m_llAbsExpireTime = 0;
    }
    else
    {
        if (m_iMasterNodeID != oVariables.masternodeid())
        {
            bMasterChange = true;
        }
        m_iMasterNodeID = oVariables.masternodeid();
        m_llAbsExpireTime = Time::GetSteadyClockMS() + oVariables.leasetime();
    }

    if (bMasterChange)
    {
        if (m_pMasterChangeCallback != nullptr)
        {
            m_pMasterChangeCallback(m_iMyGroupIdx, NodeInfo(m_iMasterNodeID), m_llMasterVersion);
        }
    }

    return 0;
}

////////////////////////////////////////////////////////

void MasterStateMachine :: BeforePropose(const int iGroupIdx, std::string & sValue)
{
    std::lock_guard<std::mutex> oLockGuard(m_oMutex);
    MasterOperator oMasterOper;
    bool bSucc = oMasterOper.ParseFromArray(sValue.data(), sValue.size());
    if (!bSucc)
    {
        return;
    }

    oMasterOper.set_lastversion(m_llMasterVersion);
    sValue.clear();
    bSucc = oMasterOper.SerializeToString(&sValue);
    assert(bSucc == true);
} 

const bool MasterStateMachine :: NeedCallBeforePropose()
{
    return true;
}

}


