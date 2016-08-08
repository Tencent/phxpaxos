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

#include "event_loop.h"
#include "event_base.h"
#include "tcp_acceptor.h"
#include "tcp_client.h"
#include "comm_include.h"

using namespace std;

namespace phxpaxos
{

EventLoop :: EventLoop()
{
    m_iEpollFd = -1;
    m_bIsEnd = false;
    m_poTcpAcceptor = nullptr;
    m_poTcpClient = nullptr;
    m_poNotify = nullptr;
    memset(m_EpollEvents, 0, sizeof(m_EpollEvents));
}

EventLoop :: ~EventLoop()
{
}

void EventLoop :: JumpoutEpollWait()
{
    m_poNotify->SendNotify();
}

void EventLoop :: SetTcpAcceptor(TcpAcceptor * poTcpAcceptor)
{
    m_poTcpAcceptor = poTcpAcceptor;
}

void EventLoop :: SetTcpClient(TcpClient * poTcpClient)
{
    m_poTcpClient = poTcpClient;
}

int EventLoop :: Init(const int iEpollLength)
{
    m_iEpollFd = epoll_create(iEpollLength);
    if (m_iEpollFd == -1)
    {
        PLErr("epoll_create fail, ret %d", m_iEpollFd);
        return -1;
    }

    m_poNotify = new Notify(this);
    assert(m_poNotify != nullptr);
    
    int ret = m_poNotify->Init();
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

void EventLoop :: ModEvent(const Event * poEvent, const int iEvents)
{
    auto it = m_mapEvent.find(poEvent->GetSocketFd());
    int iEpollOpertion = 0;
    if (it == end(m_mapEvent))
    {
        iEpollOpertion = EPOLL_CTL_ADD;
    }
    else
    {
        iEpollOpertion = it->second.m_iEvents ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    }

    epoll_event tEpollEvent;
    tEpollEvent.events = iEvents;
    tEpollEvent.data.fd = poEvent->GetSocketFd();

    int ret = epoll_ctl(m_iEpollFd, iEpollOpertion, poEvent->GetSocketFd(), &tEpollEvent);
    if (ret == -1)
    {
        PLErr("epoll_ctl fail, EpollFd %d EpollOpertion %d SocketFd %d EpollEvent %d",
                m_iEpollFd, iEpollOpertion, poEvent->GetSocketFd(), iEvents);
        
        //to do 
        return;
    }

    EventCtx tCtx;
    tCtx.m_poEvent = (Event *)poEvent;
    tCtx.m_iEvents = iEvents;
    
    m_mapEvent[poEvent->GetSocketFd()] = tCtx;
}

void EventLoop :: RemoveEvent(const Event * poEvent)
{
    auto it = m_mapEvent.find(poEvent->GetSocketFd());
    if (it == end(m_mapEvent))
    {
        return;
    }

    int iEpollOpertion = EPOLL_CTL_DEL;

    epoll_event tEpollEvent;
    tEpollEvent.events = 0;
    tEpollEvent.data.fd = poEvent->GetSocketFd();

    int ret = epoll_ctl(m_iEpollFd, iEpollOpertion, poEvent->GetSocketFd(), &tEpollEvent);
    if (ret == -1)
    {
        PLErr("epoll_ctl fail, EpollFd %d EpollOpertion %d SocketFd %d",
                m_iEpollFd, iEpollOpertion, poEvent->GetSocketFd());

        //to do 
        //when error
        return;
    }

    m_mapEvent.erase(poEvent->GetSocketFd());
}

void EventLoop :: StartLoop()
{
    m_bIsEnd = false;
    while(true)
    {
        BP->GetNetworkBP()->TcpEpollLoop();

        int iNextTimeout = 1000;
        
        DealwithTimeout(iNextTimeout);

        //PLHead("nexttimeout %d", iNextTimeout);

        OneLoop(iNextTimeout);

        //deal with accept fds
        if (m_poTcpAcceptor != nullptr)
        {
            m_poTcpAcceptor->CreateEvent();
        }

        if (m_poTcpClient != nullptr)
        {
            m_poTcpClient->DealWithWrite();
        }

        if (m_bIsEnd)
        {
            PLHead("TCP.EventLoop [END]");
            break;
        }
    }
}

void EventLoop :: Stop()
{
    m_bIsEnd = true;
}

void EventLoop :: OneLoop(const int iTimeoutMs)
{
    int n = epoll_wait(m_iEpollFd, m_EpollEvents, MAX_EVENTS, 1);
    if (n == -1)
    {
        if (errno != EINTR)
        {
            PLErr("epoll_wait fail, errno %d", errno);
            return;
        }
    }

    for (int i = 0; i < n; i++)
    {
        int iFd = m_EpollEvents[i].data.fd;
        auto it = m_mapEvent.find(iFd);
        if (it == end(m_mapEvent))
        {
            continue;
        }

        int iEvents = m_EpollEvents[i].events;
        Event * poEvent = it->second.m_poEvent;

        int ret = 0;
        if (iEvents & EPOLLERR)
        {
            OnError(iEvents, poEvent);
            continue;
        }
        
        try
        {
            if (iEvents & EPOLLIN)
            {
                ret = poEvent->OnRead();
            }

            if (iEvents & EPOLLOUT)
            {
                ret = poEvent->OnWrite();
            }
        }
        catch (...)
        {
            ret = -1;
        }

        if (ret != 0)
        {
            OnError(iEvents, poEvent);
        }
    }
}

void EventLoop :: OnError(const int iEvents, Event * poEvent)
{
    BP->GetNetworkBP()->TcpOnError();

    PLErr("event error, events %d socketfd %d socket ip %s errno %d", 
            iEvents, poEvent->GetSocketFd(), poEvent->GetSocketHost().c_str(), errno);

    RemoveEvent(poEvent);

    bool bNeedDelete = false;
    poEvent->OnError(bNeedDelete);
    
    if (bNeedDelete)
    {
        poEvent->Destroy();
    }
}

bool EventLoop :: AddTimer(const Event * poEvent, const int iTimeout, const int iType, uint32_t & iTimerID)
{
    if (poEvent->GetSocketFd() == 0)
    {
        return false;
    }    

    if (m_mapEvent.find(poEvent->GetSocketFd()) == end(m_mapEvent))
    {
        EventCtx tCtx;
        tCtx.m_poEvent = (Event *)poEvent;
        tCtx.m_iEvents = 0;

        m_mapEvent[poEvent->GetSocketFd()] = tCtx;
    }

    uint64_t llAbsTime = Time::GetSteadyClockMS() + iTimeout;
    m_oTimer.AddTimerWithType(llAbsTime, iType, iTimerID);

    m_mapTimerID2FD[iTimerID] = poEvent->GetSocketFd();

    return true;
}

void EventLoop :: RemoveTimer(const uint32_t iTimerID)
{
    auto it = m_mapTimerID2FD.find(iTimerID);
    if (it != end(m_mapTimerID2FD))
    {
        m_mapTimerID2FD.erase(it);
    }
}

void EventLoop :: DealwithTimeoutOne(const uint32_t iTimerID, const int iType)
{
    auto it = m_mapTimerID2FD.find(iTimerID);
    if (it == end(m_mapTimerID2FD))
    {
        //PLErr("Timeout aready remove!, timerid %u iType %d", iTimerID, iType);
        return;
    }

    int iSocketFd = it->second;

    m_mapTimerID2FD.erase(it);

    auto eventIt = m_mapEvent.find(iSocketFd);
    if (eventIt == end(m_mapEvent))
    {
        return;
    }

    eventIt->second.m_poEvent->OnTimeout(iTimerID, iType);
}

void EventLoop :: DealwithTimeout(int & iNextTimeout)
{
    bool bHasTimeout = true;

    while(bHasTimeout)
    {
        uint32_t iTimerID = 0;
        int iType = 0;
        bHasTimeout = m_oTimer.PopTimeout(iTimerID, iType);

        if (bHasTimeout)
        {
            DealwithTimeoutOne(iTimerID, iType);

            iNextTimeout = m_oTimer.GetNextTimeout();
            if (iNextTimeout != 0)
            {
                break;
            }
        }
    }
}

}


