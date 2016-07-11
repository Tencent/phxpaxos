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

#include "commdef.h"
#include "sm_base.h"
#include <string.h>

using namespace std;

namespace phxpaxos
{

SMFac :: SMFac()
{
}

SMFac :: ~SMFac()
{
}

bool SMFac :: DoExecute(StateMachine * poSM, const int iGroupIdx, 
        const uint64_t llInstanceID, const std::string & sPaxosValue, SMCtx * poSMCtx)
{
    std::string sBodyValue = string(sPaxosValue.data() + sizeof(int), sPaxosValue.size() - sizeof(int));
    return poSM->Execute(iGroupIdx, llInstanceID, sBodyValue, poSMCtx);
}

bool SMFac :: Execute(const int iGroupIdx, const uint64_t llInstanceID, const std::string & sPaxosValue, SMCtx * poSMCtx)
{
    if (sPaxosValue.size() < sizeof(int))
    {
        PLErr("Value wrong, instanceid %lu size %zu", llInstanceID, sPaxosValue.size());
        //need do nothing, just skip
        return true;
    }

    int iSMID = 0;
    memcpy(&iSMID, sPaxosValue.data(), sizeof(int));

    if (iSMID == 0)
    {
        PLImp("Value no need to do sm, just skip, instanceid %lu", llInstanceID);
        return true;
    }

    if (m_vecSMList.size() == 0)
    {
        PLImp("No any sm, need wait sm, instanceid %lu", llInstanceID);
        return false;
    }

    for (auto & poSM : m_vecSMList)
    {
        if (poSM->SMID() == iSMID)
        {
            return DoExecute(poSM, iGroupIdx, llInstanceID, sPaxosValue, poSMCtx);
        }
    }

    PLErr("Unknown smid %d instanceid %lu", iSMID, llInstanceID);

    return false;
}

bool SMFac :: ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, const std::string & sPaxosValue)
{
    if (sPaxosValue.size() < sizeof(int))
    {
        PLErr("Value wrong, instanceid %lu size %zu", llInstanceID, sPaxosValue.size());
        //need do nothing, just skip
        return true;
    }

    int iSMID = 0;
    memcpy(&iSMID, sPaxosValue.data(), sizeof(int));

    if (iSMID == 0)
    {
        PLImp("Value no need to do sm, just skip, instanceid %lu", llInstanceID);
        return true;
    }

    if (m_vecSMList.size() == 0)
    {
        PLImp("No any sm, need wait sm, instanceid %lu", llInstanceID);
        return false;
    }

    for (auto & poSM : m_vecSMList)
    {
        if (poSM->SMID() == iSMID)
        {
            std::string sBodyValue = string(sPaxosValue.data() + sizeof(int), sPaxosValue.size() - sizeof(int));
            return poSM->ExecuteForCheckpoint(iGroupIdx, llInstanceID, sBodyValue);
        }
    }

    PLErr("Unknown smid %d instanceid %lu", iSMID, llInstanceID);

    return false;
}


void SMFac :: PackPaxosValue(std::string & sPaxosValue, const int iSMID)
{
    char sSMID[sizeof(int)] = {0};
    if (iSMID != 0)
    {
        memcpy(sSMID, &iSMID, sizeof(sSMID));
    }

    sPaxosValue = string(sSMID, sizeof(sSMID)) + sPaxosValue;
}

void SMFac :: AddSM(StateMachine * poSM)
{
    for (auto & poSMt : m_vecSMList)
    {
        if (poSMt->SMID() == poSM->SMID())
        {
            return;
        }
    }

    m_vecSMList.push_back(poSM);
}

///////////////////////////////////////////////////////

const uint64_t SMFac :: GetCheckpointInstanceID(const int iGroupIdx) const
{
    uint64_t llCPInstanceID = -1;
    uint64_t llCPInstanceID_Insize = -1;
    bool bHaveUseSM = false;

    for (auto & poSM : m_vecSMList)
    {
        uint64_t llCheckpointInstanceID = poSM->GetCheckpointInstanceID(iGroupIdx);
        if (poSM->SMID() == SYSTEM_V_SMID
                || poSM->SMID() == MASTER_V_SMID)
        {
            //system variables 
            //master variables
            //if no user state machine, system and master's can use.
            //if have user state machine, use user'state machine's checkpointinstanceid.
            if (llCheckpointInstanceID == uint64_t(-1))
            {
                continue;
            }
            
            if (llCheckpointInstanceID > llCPInstanceID_Insize
                    || llCPInstanceID_Insize == (uint64_t)-1)
            {
                llCPInstanceID_Insize = llCheckpointInstanceID;
            }

            continue;
        }

        bHaveUseSM = true;

        if (llCheckpointInstanceID == uint64_t(-1))
        {
            continue;
        }
        
        if (llCheckpointInstanceID > llCPInstanceID
                || llCPInstanceID == (uint64_t)-1)
        {
            llCPInstanceID = llCheckpointInstanceID;
        }
    }
    
    return bHaveUseSM ? llCPInstanceID : llCPInstanceID_Insize;
}

std::vector<StateMachine *> SMFac :: GetSMList()
{
    return m_vecSMList;
}

}


