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

#include "dfnetwork.h"
#include "udp.h"

namespace phxpaxos 
{

DFNetWork :: DFNetWork() : m_oUDPRecv(this), m_oTcpIOThread(this)
{
}

DFNetWork :: ~DFNetWork()
{
    PLHead("NetWork Deleted!");
}

void DFNetWork :: StopNetWork()
{
    m_oUDPRecv.Stop();
    m_oUDPSend.Stop();
    m_oTcpIOThread.Stop();
}

int DFNetWork :: Init(const std::string & sListenIp, const int iListenPort) 
{
    int ret = m_oUDPSend.Init();
    if (ret != 0)
    {
        return ret;
    }

    ret = m_oUDPRecv.Init(iListenPort);
    if (ret != 0)
    {
        return ret;
    }

    ret = m_oTcpIOThread.Init(sListenIp, iListenPort);
    if (ret != 0)
    {
        PLErr("m_oTcpIOThread Init fail, ret %d", ret);
        return ret;
    }

    return 0;
}

void DFNetWork :: RunNetWork()
{
    m_oUDPSend.start();
    m_oUDPRecv.start();
    m_oTcpIOThread.Start();
}

int DFNetWork :: SendMessageTCP(const std::string & sIp, const int iPort, const std::string & sMessage)
{
    return m_oTcpIOThread.AddMessage(sIp, iPort, sMessage);
}

int DFNetWork :: SendMessageUDP(const std::string & sIp, const int iPort, const std::string & sMessage)
{
    return m_oUDPSend.AddMessage(sIp, iPort, sMessage);
}

}


