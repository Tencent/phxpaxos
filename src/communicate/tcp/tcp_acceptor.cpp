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

#include <stdio.h>
#include "tcp_acceptor.h"
#include "commdef.h"
#include "event_loop.h"
#include "phxpaxos/network.h"
#include "message_event.h"
#include "comm_include.h"
#include <poll.h>

namespace phxpaxos
{

TcpAcceptor :: TcpAcceptor()
{
    m_bIsEnd = false;
    m_bIsStarted = false;
}

TcpAcceptor :: ~TcpAcceptor()
{
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

                AddEvent(fd, oAddr);
            }
        }

        if (m_bIsEnd)
        {
            PLHead("TCP.Acceptor [END]");
            return;
        }
    }
}

void TcpAcceptor :: AddEventLoop(EventLoop * poEventLoop)
{
    m_vecEventLoop.push_back(poEventLoop);
}

void TcpAcceptor :: AddEvent(int iFD, SocketAddress oAddr)
{
    EventLoop * poMinActiveEventLoop = nullptr;
    int iMinActiveEventCount = 1 << 30;

    for (auto & poEventLoop : m_vecEventLoop)
    {
        int iActiveCount = poEventLoop->GetActiveEventCount();
        if (iActiveCount < iMinActiveEventCount)
        {
            iMinActiveEventCount = iActiveCount;
            poMinActiveEventLoop = poEventLoop;
        }
    }

    //printf("this %p addevent %p fd %d ip %s port %d\n", 
            //this, poMinActiveEventLoop, iFD, oAddr.getHost().c_str(), oAddr.getPort());
    poMinActiveEventLoop->AddEvent(iFD, oAddr);
}
    
}


