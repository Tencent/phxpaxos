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
#include "udp.h"
#include "tcp.h"
#include "phxpaxos/network.h"

namespace phxpaxos 
{

class DFNetWork : public NetWork
{
public:
    DFNetWork();
    virtual ~DFNetWork();

    int Init(const std::string & sListenIp, const int iListenPort);

    void RunNetWork();

    void StopNetWork();

    int SendMessageTCP(const std::string & sIp, const int iPort, const std::string & sMessage);

    int SendMessageUDP(const std::string & sIp, const int iPort, const std::string & sMessage);

private:
    UDPRecv m_oUDPRecv;
    UDPSend m_oUDPSend;
    TcpIOThread m_oTcpIOThread;
};

}
