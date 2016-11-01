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
#include <condition_variable>

namespace phxpaxos
{

class SerialLock
{
public:
    SerialLock();
    ~SerialLock();

    void Lock();
    void UnLock();

    void Wait();
    void Interupt();

    bool WaitTime(const int iTimeMs);

private:
    std::mutex m_oMutex;
    std::unique_lock<std::mutex> m_oLock;
    std::condition_variable m_oCond;
};

}
