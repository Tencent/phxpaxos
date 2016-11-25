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

#include "utils_include.h"

namespace phxpaxos 
{

class DFNetWork;

class UDPRecv : public Thread
{
public:
    UDPRecv(DFNetWork * poDFNetWork);
    ~UDPRecv();

    int Init(const int iPort);

    void run();

    void Stop();
    
private:
    DFNetWork * m_poDFNetWork;
    int m_iSockFD;
    bool m_bIsEnd;
    bool m_bIsStarted;
};

class UDPSend : public Thread
{
public:
    UDPSend();
    ~UDPSend();

    int Init();

    void run();

    void Stop();

    int AddMessage(const std::string & sIP, const int iPort, const std::string & sMessage);

    struct QueueData
    {
        std::string m_sIP;
        int m_iPort;
        std::string m_sMessage;
    };

private:
    void SendMessage(const std::string & sIP, const int iPort, const std::string & sMessage);

private:
    Queue<QueueData *> m_oSendQueue;
    int m_iSockFD;
    bool m_bIsEnd;
    bool m_bIsStarted;
};
    
}
