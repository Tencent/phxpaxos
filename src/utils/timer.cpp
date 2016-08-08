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

#include "timer.h"
#include "util.h"
#include <algorithm> 

namespace phxpaxos
{

Timer :: Timer() : m_iNowTimerID(1)
{
}

Timer :: ~Timer()
{
}

void Timer :: AddTimer(const uint64_t llAbsTime, uint32_t & iTimerID)
{
    return AddTimerWithType(llAbsTime, 0, iTimerID);
}

void Timer :: AddTimerWithType(const uint64_t llAbsTime, const int iType, uint32_t & iTimerID)
{
    iTimerID = m_iNowTimerID++;

    TimerObj tObj(iTimerID, llAbsTime, iType);
    m_vecTimerHeap.push_back(tObj);
    push_heap(begin(m_vecTimerHeap), end(m_vecTimerHeap));
}

const int Timer :: GetNextTimeout() const
{
    if (m_vecTimerHeap.empty())
    {
        return -1;
    }

    int iNextTimeout = 0;

    TimerObj tObj = m_vecTimerHeap.front();
    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (tObj.m_llAbsTime > llNowTime)
    {
        iNextTimeout = (int)(tObj.m_llAbsTime - llNowTime);
    }

    return iNextTimeout;
}

bool Timer :: PopTimeout(uint32_t & iTimerID, int & iType)
{
    if (m_vecTimerHeap.empty())
    {
        return false;
    }

    TimerObj tObj = m_vecTimerHeap.front();
    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (tObj.m_llAbsTime > llNowTime)
    {
        return false;
    }
    
    pop_heap(begin(m_vecTimerHeap), end(m_vecTimerHeap));
    m_vecTimerHeap.pop_back();

    iTimerID = tObj.m_iTimerID;
    iType = tObj.m_iType;

    return true;
}    
    
}


