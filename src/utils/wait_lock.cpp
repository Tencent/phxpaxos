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

#include "wait_lock.h"

namespace phxpaxos
{

WaitLock :: WaitLock() : m_bIsLockUsing(false)
{
}

WaitLock :: ~WaitLock()
{
}

bool WaitLock :: Lock(const int iTimeoutMs, int & iUseTimeMs)
{
    uint64_t llBeginTime = Time::GetTimestampMS();

    m_oSerialLock.Lock();

    while (m_bIsLockUsing)
    {
        if (iTimeoutMs == -1)
        {
            m_oSerialLock.Wait();
        }
        else
        {
            if (!m_oSerialLock.WaitTime(iTimeoutMs))
            {
                //lock timeout
                iUseTimeMs = iTimeoutMs;
                m_oSerialLock.UnLock();

                return false;
            }
        }
    }

    m_bIsLockUsing = true;
    m_oSerialLock.UnLock();

    uint64_t llEndTime = Time::GetTimestampMS();
    iUseTimeMs = llEndTime > llBeginTime ? (int)(llEndTime - llBeginTime) : 0;

    return true;
}

void WaitLock :: UnLock()
{
    m_oSerialLock.Lock();

    m_bIsLockUsing = false;
    m_oSerialLock.Interupt();

    m_oSerialLock.UnLock();
}

}

