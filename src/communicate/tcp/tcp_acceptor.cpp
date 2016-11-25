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

#include "tcp_acceptor.h"
#include "commdef.h"
#include "event_loop.h"
#include "phxpaxos/network.h"
#include "message_event.h"
#include "comm_include.h"
#include <poll.h>

namespace phxpaxos
{

TcpAcceptor :: TcpAcceptor(
        EventLoop * poEventLoop,
        NetWork * poNetWork)
    : m_poEventLoop(poEventLoop),
    m_poNetWork(poNetWork)
{
    m_bIsEnd = false;
    m_bIsStarted = false;
}

TcpAcceptor :: ~TcpAcceptor()
{
    while (!m_oFDQueue.empty())
    {
        AcceptData * poData = m_oFDQueue.front();
        m_oFDQueue.pop();

        delete poData;
    }

    ClearEvent();
}

void TcpAcceptor :: Listen(const std::string & sListenIP, const int iListenPort)
{
    m_oSocket.listen(SocketAddress(sListenIP, (unsigned short)iListenPort));
}

void TcpAcceptor :: Stop()
{
    if (m_bIsStarted)
    {
        m_bIsEnd = true;
        join();
    }
}

void TcpAcceptor :: run()
{
    m_bIsStarted = true;

    PLHead("start accept...");

    m_oSocket.setAcceptTimeout(500);
    m_oSocket.setNonBlocking(true);

    while (true)
    {
        struct pollfd pfd;
        int ret;

        pfd.fd =  m_oSocket.getSocketHandle();
        pfd.events = POLLIN;
        ret = poll(&pfd, 1, 500);

        if (ret != 0 && ret != -1)
        {
            SocketAddress oAddr;
            int fd = -1;
            try
            {
                fd = m_oSocket.acceptfd(&oAddr);
            }
            catch(...)
            {
                fd = -1;
            }
            
            if (fd >= 0)
            {
                BP->GetNetworkBP()->TcpAcceptFd();

                PLImp("accepted!, fd %d ip %s port %d",
                        fd, oAddr.getHost().c_str(), oAddr.getPort());
                AcceptData * poData = new AcceptData;
                poData->fd = fd;
                poData->oAddr = oAddr;
                
                m_oMutex.lock();
                m_oFDQueue.push(poData);
                m_oMutex.unlock();
            }
        }

        ClearEvent();

        if (m_bIsEnd)
        {
            PLHead("TCP.Acceptor [END]");
            return;
        }
    }
}

void TcpAcceptor :: CreateEvent()
{
    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    if (m_oFDQueue.empty())
    {
        return;
    }
    
    int iCreatePerTime = 200;
    while ((!m_oFDQueue.empty()) && iCreatePerTime--)
    {
        AcceptData * poData = m_oFDQueue.front();
        m_oFDQueue.pop();

        //create event for this fd
        MessageEvent * poMessageEvent = new MessageEvent(MessageEventType_RECV, poData->fd, 
                poData->oAddr, m_poEventLoop, m_poNetWork);
        poMessageEvent->AddEvent(EPOLLIN);

        m_vecCreatedEvent.push_back(poMessageEvent);

        delete poData;
    }
}

void TcpAcceptor :: ClearEvent()
{
    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    for (auto it = m_vecCreatedEvent.begin(); it != end(m_vecCreatedEvent);)
    {
        if ((*it)->IsDestroy())
        {
            delete (*it);
            it = m_vecCreatedEvent.erase(it);
        }
        else
        {
            it++;
        }
    }
}
    
}


