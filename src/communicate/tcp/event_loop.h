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

#include <map>
#include <sys/epoll.h>
#include "timer.h"
#include "notify.h"

namespace phxpaxos
{

#define MAX_EVENTS 1024

class Event;
class TcpAcceptor;
class TcpClient;

class EventLoop
{
public:
    EventLoop();
    virtual ~EventLoop();

    int Init(const int iEpollLength);

    void ModEvent(const Event * poEvent, const int iEvents);

    void RemoveEvent(const Event * poEvent);

    void StartLoop();

    void Stop();

    void OnError(const int iEvents, Event * poEvent);

    virtual void OneLoop(const int iTimeoutMs);

public:
    void SetTcpAcceptor(TcpAcceptor * poTcpAcceptor);

    void SetTcpClient(TcpClient * poTcpClient);

    void JumpoutEpollWait();

public:
    bool AddTimer(const Event * poEvent, const int iTimeout, const int iType, uint32_t & iTimerID);

    void RemoveTimer(const uint32_t iTimerID);

    void DealwithTimeout(int & iNextTimeout);

    void DealwithTimeoutOne(const uint32_t iTimerID, const int iType);

public:
    typedef struct EventCtx
    {
        Event * m_poEvent;
        int m_iEvents;
    } EventCtx_t;

private:
    bool m_bIsEnd;

protected:
    int m_iEpollFd;
    epoll_event m_EpollEvents[MAX_EVENTS];
    std::map<int, EventCtx_t> m_mapEvent;
    TcpAcceptor * m_poTcpAcceptor;
    TcpClient * m_poTcpClient;
    Notify * m_poNotify;

protected:
    Timer m_oTimer;
    std::map<uint32_t, int> m_mapTimerID2FD;
};
    
}
