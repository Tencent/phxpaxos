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

#include "tcp_client.h"
#include "commdef.h"
#include "phxpaxos/network.h"
#include "event_loop.h"

namespace phxpaxos
{

TcpClient :: TcpClient(EventLoop * poEventLoop, NetWork * poNetWork)
    : m_poEventLoop(poEventLoop), m_poNetWork(poNetWork)
{
}

TcpClient :: ~TcpClient()
{
    for (auto & it : m_mapEvent)
    {
        delete it.second;
    }
}

int TcpClient :: AddMessage(const std::string & sIP, const int iPort, const std::string & sMessage)
{
    //PLImp("ok");
    MessageEvent * poEvent = GetEvent(sIP, iPort);
    if (poEvent == nullptr)
    {
        PLErr("no event created for this ip %s port %d", sIP.c_str(), iPort);
        return -1;
    }

    return poEvent->AddMessage(sMessage);
}

MessageEvent * TcpClient :: GetEvent(const std::string & sIP, const int iPort)
{
    uint32_t iIP = (uint32_t)inet_addr(sIP.c_str());
    uint64_t llNodeID = (((uint64_t)iIP) << 32) | iPort;

    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    auto it = m_mapEvent.find(llNodeID);
    if (it != end(m_mapEvent))
    {
        return it->second;
    }

    return CreateEvent(llNodeID, sIP, iPort);
}

MessageEvent * TcpClient :: CreateEvent(const uint64_t llNodeID, const std::string & sIP, const int iPort)
{
    PLImp("start, ip %s port %d", sIP.c_str(), iPort);

    Socket oSocket;
    oSocket.setNonBlocking(true);
    oSocket.setNoDelay(true);
    SocketAddress oAddr(sIP, iPort);
    oSocket.connect(oAddr);

    MessageEvent * poEvent = new MessageEvent(MessageEventType_SEND, oSocket.detachSocketHandle(), 
            oAddr, m_poEventLoop, m_poNetWork);
    assert(poEvent != nullptr);

    m_mapEvent[llNodeID] = poEvent;
    m_vecEvent.push_back(poEvent);

    PLImp("ok, ip %s port %d", sIP.c_str(), iPort);

    return poEvent;
}

void TcpClient :: DealWithWrite()
{
    size_t iSize = m_vecEvent.size();

    for (size_t i = 0; i < iSize; i++)
    {
        m_vecEvent[i]->OpenWrite();
    }
    //PLImp("end, live event count %zu", vecEventList.size());
}
    
}


