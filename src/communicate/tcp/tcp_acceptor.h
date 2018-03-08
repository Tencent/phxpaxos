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

#include <vector>
#include <mutex>
#include "utils_include.h"
#include "message_event.h"

namespace phxpaxos
{

class EventLoop;
class NetWork;

class TcpAcceptor : public Thread
{
public:
    TcpAcceptor();
    ~TcpAcceptor();

    void Listen(const std::string & sListenIP, const int iListenPort);

    void run();

    void Stop();

    void AddEventLoop(EventLoop * poEventLoop);

    void AddEvent(int iFD, SocketAddress oAddr);

private:
    ServerSocket m_oSocket;
    std::vector<EventLoop *> m_vecEventLoop;

private:
    bool m_bIsEnd;
    bool m_bIsStarted;
};
    
}
