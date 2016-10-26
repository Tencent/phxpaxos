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

#include "notifier_pool.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

namespace phxpaxos
{

Notifier :: Notifier()
{
    m_iPipeFD[0] = -1;
    m_iPipeFD[1] = -1;
}

Notifier :: ~Notifier()
{
    for (int i = 0; i < 2; i++)
    {   
        if (m_iPipeFD[i] != -1) 
        {   
            close(m_iPipeFD[i]);
        }   
    }
}

int Notifier :: Init()
{
    int ret = pipe(m_iPipeFD);
    if (ret != 0)
    {   
        return ret;
    } 

    return 0;
}

void Notifier :: SendNotify(const int ret)
{
    int iWriteLen = write(m_iPipeFD[1], (char *)&ret, sizeof(int));
    assert(iWriteLen == sizeof(int));
}

void Notifier :: WaitNotify(int & ret)
{
    ret = -1;
    int iReadLen = read(m_iPipeFD[0], (char *)&ret, sizeof(int));
    assert(iReadLen == sizeof(int));
}

///////////////////////////////////

NotifierPool :: NotifierPool()
{
}

NotifierPool :: ~NotifierPool()
{
    for (auto & it : m_mapPool)
    {
        delete it.second;
    }
}

int NotifierPool :: GetNotifier(const uint64_t iID, Notifier *& poNotifier)
{
    poNotifier = nullptr;
    std::lock_guard<std::mutex> oLock(m_oMutex);

    auto it = m_mapPool.find(iID);
    if (it != end(m_mapPool))
    {
        poNotifier = it->second;
        return 0;
    }

    poNotifier = new Notifier();
    assert(poNotifier != nullptr);
    int ret = poNotifier->Init();
    if (ret != 0)
    {
        return ret;
    }

    m_mapPool[iID] = poNotifier;
    return 0;
}

}

