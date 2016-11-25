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

#include "udp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dfnetwork.h"
#include "comm_include.h"
#include <poll.h>

namespace phxpaxos 
{

UDPRecv :: UDPRecv(DFNetWork * poDFNetWork) 
    : m_poDFNetWork(poDFNetWork), m_iSockFD(-1), m_bIsEnd(false), m_bIsStarted(false)
{
}

UDPRecv :: ~UDPRecv()
{
    if (m_iSockFD != -1)
    {
        close(m_iSockFD);
        m_iSockFD = -1;
    }
}

void UDPRecv :: Stop()
{
    if (m_bIsStarted)
    {
        m_bIsEnd = true;
        join();
    }
}

int UDPRecv :: Init(const int iPort)
{
    if ((m_iSockFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(iPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int enable = 1;
    setsockopt(m_iSockFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(m_iSockFD, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
    {
        return -1;
    }

    return 0;
}

void UDPRecv :: run()
{
    m_bIsStarted = true;

    char sBuffer[65536] = {0};

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    memset(&addr, 0, sizeof(addr));

    while(true)
    {
        if (m_bIsEnd)
        {
            PLHead("UDPRecv [END]");
            return;
        }

        struct pollfd fd;
        int ret;

        fd.fd = m_iSockFD;
        fd.events = POLLIN;
        ret = poll(&fd, 1, 500);

        if (ret == 0 || ret == -1)
        {
            continue;
        }
        
        int iRecvLen = recvfrom(m_iSockFD, sBuffer, sizeof(sBuffer), 0,
                (struct sockaddr *)&addr, &addr_len);

        //printf("recvlen %d, buffer %s client %s\n",
                //iRecvLen, sBuffer, inet_ntoa(addr.sin_addr));
        
        BP->GetNetworkBP()->UDPReceive(iRecvLen);

        if (iRecvLen > 0)
        {
            m_poDFNetWork->OnReceiveMessage(sBuffer, iRecvLen);
        }
    }
}

//////////////////////////////////////////////

UDPSend :: UDPSend() : m_iSockFD(-1), m_bIsEnd(false), m_bIsStarted(false)
{
}

UDPSend :: ~UDPSend()
{
    while (!m_oSendQueue.empty())
    {
        QueueData * poData = m_oSendQueue.peek();
        m_oSendQueue.pop();
        delete poData;
    }
}

int UDPSend :: Init()
{
    if ((m_iSockFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        return -1;
    }

    return 0;
}

void UDPSend :: Stop()
{
    if (m_bIsStarted)
    {
        m_bIsEnd = true;
        join();
    }
}

void UDPSend :: SendMessage(const std::string & sIP, const int iPort, const std::string & sMessage)
{
    struct sockaddr_in addr;
    int addr_len = sizeof(struct sockaddr_in);
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(iPort);
    addr.sin_addr.s_addr = inet_addr(sIP.c_str());
    
    int ret = sendto(m_iSockFD, sMessage.data(), (int)sMessage.size(), 0, (struct sockaddr *)&addr, addr_len);
    if (ret > 0)
    {
        BP->GetNetworkBP()->UDPRealSend(sMessage);
    }
}

void UDPSend :: run()
{
    m_bIsStarted = true;

    while(true)
    {
        QueueData * poData = nullptr;

        m_oSendQueue.lock();

        bool bSucc = m_oSendQueue.peek(poData, 1000);
        if (bSucc)
        {
            m_oSendQueue.pop();
        }

        m_oSendQueue.unlock();

        if (poData != nullptr)
        {
            SendMessage(poData->m_sIP, poData->m_iPort, poData->m_sMessage);
            delete poData;
        }

        if (m_bIsEnd)
        {
            PLHead("UDPSend [END]");
            return;
        }
    }
}

int UDPSend :: AddMessage(const std::string & sIP, const int iPort, const std::string & sMessage)
{
    m_oSendQueue.lock();

    if ((int)m_oSendQueue.size() > UDP_QUEUE_MAXLEN)
    {
        BP->GetNetworkBP()->UDPQueueFull();
        //PLErr("queue length %d too long, can't enqueue", m_oSendQueue.size());

        m_oSendQueue.unlock();

        return -2;
    }

    QueueData * poData = new QueueData;
    poData->m_sIP = sIP;
    poData->m_iPort = iPort;
    poData->m_sMessage = sMessage;

    m_oSendQueue.add(poData);
    m_oSendQueue.unlock();

    return 0;
}
    
}


