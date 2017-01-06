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

#include "event_loop.h"
#include "tcp_acceptor.h"
#include "tcp_client.h"
#include "utils_include.h"

namespace phxpaxos
{

class TcpRead : public Thread
{
public:
    TcpRead(NetWork * poNetWork);
    ~TcpRead();

    int Init(const std::string & sListenIp, const int iListenPort);
    
    void run();

    void Stop();

private:
    TcpAcceptor m_oTcpAcceptor;
    EventLoop m_oEventLoop;
};

/////////////////////////////////////////////

class TcpWrite : public Thread
{
public:
    TcpWrite(NetWork * poNetWork);
    ~TcpWrite();

    int Init();

    void run();

    void Stop();

    int AddMessage(const std::string & sIP, const int iPort, const std::string & sMessage);

private:
    TcpClient m_oTcpClient;
    EventLoop m_oEventLoop;
};

class TcpIOThread 
{
public:
    TcpIOThread(NetWork * poNetWork);
    ~TcpIOThread();

    int Init(const std::string & sListenIp, const int iListenPort);

    void Start();

    void Stop();

    int AddMessage(const std::string & sIP, const int iPort, const std::string & sMessage);

private:
    TcpRead m_oTcpRead;
    TcpWrite m_oTcpWrite;
    bool m_bIsStarted;
};
    
}
