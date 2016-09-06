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

#include "serial_lock.h"

namespace phxpaxos
{

#define WAIT_LOCK_USERTIME_AVG_INTERVAL 250

class WaitLock
{
public:
    WaitLock();
    ~WaitLock();

    bool Lock(const int iTimeoutMs, int & iUseTimeMs);

    void UnLock();

    void SetMaxWaitLogCount(const int iMaxWaitLockCount);

    void SetLockWaitTimeThreshold(const int iLockWaitTimeThresholdMS);

public:
    //stat
    int GetNowHoldThreadCount();

    int GetNowAvgThreadWaitTime();

    int GetNowRejectRate();

private:
    void RefleshRejectRate(const int iUseTimeMs);

    bool CanLock();

private:
    SerialLock m_oSerialLock;
    bool m_bIsLockUsing;

    int m_iWaitLockCount;
    int m_iMaxWaitLockCount;

    int m_iLockUseTimeSum;
    int m_iAvgLockUseTime;
    int m_iLockUseTimeCount;

    int m_iRejectRate;
    int m_iLockWaitTimeThresholdMS;
};
    
}
