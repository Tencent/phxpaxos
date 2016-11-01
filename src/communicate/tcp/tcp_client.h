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

#include <string>
#include <map>
#include <mutex>
#include "message_event.h"
#include "utils_include.h"

namespace phxpaxos
{

class EventLoop;
class NetWork;

class TcpClient
{
public:
    TcpClient(
            EventLoop * poEventLoop,
            NetWork * poNetWork);
    ~TcpClient();

    int AddMessage(const std::string & sIP, const int iPort, const std::string & sMessage);

    void DealWithWrite();

private:
    MessageEvent * GetEvent(const std::string & sIP, const int iPort);
    
    MessageEvent * CreateEvent(const uint64_t llNodeID, const std::string & sIP, const int iPort);

private:
    EventLoop * m_poEventLoop;
    NetWork * m_poNetWork;

private:
    std::map<uint64_t, MessageEvent *> m_mapEvent;
    std::vector<MessageEvent *> m_vecEvent;
    std::mutex m_oMutex;
    
};
    
}
