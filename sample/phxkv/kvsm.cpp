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

#include "kvsm.h"
#include "phxkv.pb.h"
#include "log.h"
#include <algorithm>

using namespace phxpaxos;
using namespace std;

namespace phxkv
{

PhxKVSM :: PhxKVSM(const std::string & sDBPath)
    : m_llCheckpointInstanceID(phxpaxos::NoCheckpoint), m_iSkipSyncCheckpointTimes(0)
{
    m_sDBPath = sDBPath;
}

PhxKVSM :: ~PhxKVSM()
{
}

const bool PhxKVSM :: Init()
{
    bool bSucc = m_oKVClient.Init(m_sDBPath);
    if (!bSucc)
    {
        PLErr("KVClient.Init fail, dbpath %s", m_sDBPath.c_str());
        return false;
    }

    int ret = m_oKVClient.GetCheckpointInstanceID(m_llCheckpointInstanceID);
    if (ret != 0 && ret != KVCLIENT_KEY_NOTEXIST)
    {
        PLErr("KVClient.GetCheckpointInstanceID fail, ret %d", ret);
        return false;
    }

    if (ret == KVCLIENT_KEY_NOTEXIST)
    {
        PLImp("no checkpoint");
        m_llCheckpointInstanceID = phxpaxos::NoCheckpoint;
    }
    else
    {
        PLImp("CheckpointInstanceID %lu", m_llCheckpointInstanceID);
    }

    return true;
}

int PhxKVSM :: SyncCheckpointInstanceID(const uint64_t llInstanceID)
{
    if (m_iSkipSyncCheckpointTimes++ < 5)
    {
        PLDebug("no need to sync checkpoint, skiptimes %d", m_iSkipSyncCheckpointTimes);
        return 0;
    }

    int ret = m_oKVClient.SetCheckpointInstanceID(llInstanceID);
    if (ret != 0)
    {
        PLErr("KVClient::SetCheckpointInstanceID fail, ret %d instanceid %lu", ret, llInstanceID);
        return ret;
    }

    PLImp("ok, old checkpoint instanceid %lu new checkpoint instanceid %lu",
            m_llCheckpointInstanceID, llInstanceID);

    m_llCheckpointInstanceID = llInstanceID;
    m_iSkipSyncCheckpointTimes = 0;

    return 0;
}

bool PhxKVSM :: Execute(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sPaxosValue, SMCtx * poSMCtx)
{
    KVOperator oKVOper;
    bool bSucc = oKVOper.ParseFromArray(sPaxosValue.data(), sPaxosValue.size());
    if (!bSucc)
    {
        PLErr("oKVOper data wrong");
        //wrong oper data, just skip, so return true
        return true;
    }

    int iExecuteRet = -1;
    string sReadValue;
    uint64_t llReadVersion;

    if (oKVOper.operator_() == KVOperatorType_READ)
    {
        iExecuteRet = m_oKVClient.Get(oKVOper.key(), sReadValue, llReadVersion);
    }
    else if (oKVOper.operator_() == KVOperatorType_WRITE)
    {
        iExecuteRet = m_oKVClient.Set(oKVOper.key(), oKVOper.value(), oKVOper.version());
    }
    else if (oKVOper.operator_() == KVOperatorType_DELETE)
    {
        iExecuteRet = m_oKVClient.Del(oKVOper.key(), oKVOper.version());
    }
    else
    {
        PLErr("unknown op %u", oKVOper.operator_());
        //wrong op, just skip, so return true;
        return true;
    }

    if (iExecuteRet == KVCLIENT_SYS_FAIL)
    {
        //need retry
        return false;
    }
    else
    {
        if (poSMCtx != nullptr && poSMCtx->m_pCtx != nullptr)
        {
            PhxKVSMCtx * poPhxKVSMCtx = (PhxKVSMCtx *)poSMCtx->m_pCtx;
            poPhxKVSMCtx->iExecuteRet = iExecuteRet;
            poPhxKVSMCtx->sReadValue = sReadValue;
            poPhxKVSMCtx->llReadVersion = llReadVersion;
        }

        SyncCheckpointInstanceID(llInstanceID);

        return true;
    }
}

////////////////////////////////////////////////////

bool PhxKVSM :: MakeOpValue(
        const std::string & sKey, 
        const std::string & sValue, 
        const uint64_t llVersion, 
        const KVOperatorType iOp,
        std::string & sPaxosValue)
{
    KVOperator oKVOper;
    oKVOper.set_key(sKey);
    oKVOper.set_value(sValue);
    oKVOper.set_version(llVersion);
    oKVOper.set_operator_(iOp);
    oKVOper.set_sid(rand());

    return oKVOper.SerializeToString(&sPaxosValue);
}

bool PhxKVSM :: MakeGetOpValue(
        const std::string & sKey,
        std::string & sPaxosValue)
{
    return MakeOpValue(sKey, "", 0, KVOperatorType_READ, sPaxosValue);
}

bool PhxKVSM :: MakeSetOpValue(
        const std::string & sKey, 
        const std::string & sValue, 
        const uint64_t llVersion, 
        std::string & sPaxosValue)
{
    return MakeOpValue(sKey, sValue, llVersion, KVOperatorType_WRITE, sPaxosValue);
}

bool PhxKVSM :: MakeDelOpValue(
        const std::string & sKey, 
        const uint64_t llVersion, 
        std::string & sPaxosValue)
{
    return MakeOpValue(sKey, "", llVersion, KVOperatorType_DELETE, sPaxosValue);
}

KVClient * PhxKVSM :: GetKVClient()
{
    return &m_oKVClient;
}

}

