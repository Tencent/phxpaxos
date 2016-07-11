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

#include "system_variables_store.h"
#include "db.h"

namespace phxpaxos
{

SystemVariablesStore :: SystemVariablesStore(const LogStorage * poLogStorage) : m_poLogStorage((LogStorage *)poLogStorage)
{
}

SystemVariablesStore :: ~SystemVariablesStore()
{
}

int SystemVariablesStore :: Write(const WriteOptions & oWriteOptions, const int iGroupIdx, const SystemVariables & oVariables)
{
    const int m_iMyGroupIdx = iGroupIdx;

    string sBuffer;
    bool sSucc = oVariables.SerializeToString(&sBuffer);
    if (!sSucc)
    {
        PLG1Err("Variables.Serialize fail");
        return -1;
    }
    
    int ret = m_poLogStorage->SetSystemVariables(oWriteOptions, iGroupIdx, sBuffer);
    if (ret != 0)
    {
        PLG1Err("DB.Put fail, groupidx %d bufferlen %zu ret %d", 
                iGroupIdx, sBuffer.size(), ret);
        return ret;
    }

    return 0;
}

int SystemVariablesStore :: Read(const int iGroupIdx, SystemVariables & oVariables)
{
    const int m_iMyGroupIdx = iGroupIdx;

    string sBuffer;
    int ret = m_poLogStorage->GetSystemVariables(iGroupIdx, sBuffer);
    if (ret != 0 && ret != 1)
    {
        PLG1Err("DB.Get fail, groupidx %d ret %d", iGroupIdx, ret);
        return ret;
    }
    else if (ret == 1)
    {
        PLG1Imp("DB.Get not found, groupidx %d", iGroupIdx);
        return 1;
    }

    bool bSucc = oVariables.ParseFromArray(sBuffer.data(), sBuffer.size());
    if (!bSucc)
    {
        PLG1Err("Variables.ParseFromArray fail, bufferlen %zu", sBuffer.size());
        return -1;
    }

    return 0;
}

}


