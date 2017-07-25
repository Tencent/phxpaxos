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

#include "phxpaxos/options.h"
#include "commdef.h"
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

namespace phxpaxos
{

NodeInfo :: NodeInfo()
    : m_iNodeID(nullnode), m_sIP(""), m_iPort(-1)
{
}

NodeInfo :: NodeInfo(const nodeid_t iNodeID)
    : m_iNodeID(iNodeID), m_sIP(""), m_iPort(-1)
{
    ParseNodeID();
}

NodeInfo :: NodeInfo(const std::string & sIP, const int iPort)
    : m_iNodeID(nullnode), m_sIP(sIP), m_iPort(iPort)
{
    MakeNodeID();
}

const nodeid_t NodeInfo :: GetNodeID() const
{
    return m_iNodeID;
}

const std::string & NodeInfo :: GetIP() const
{
    return m_sIP;
}

const int NodeInfo :: GetPort() const
{
    return m_iPort;
}

void NodeInfo :: SetIPPort(const std::string & sIP, const int iPort)
{
    m_sIP = sIP;
    m_iPort = iPort;
    MakeNodeID();
}

void NodeInfo :: SetNodeID(const nodeid_t iNodeID)
{
    m_iNodeID = iNodeID;
    ParseNodeID();
}

void NodeInfo :: MakeNodeID()
{
    uint32_t iIP = (uint32_t)inet_addr(m_sIP.c_str());
    assert(iIP != (uint32_t)-1);

    m_iNodeID = (((uint64_t)iIP) << 32) | m_iPort;

    //PLImp("ip %s ip %u port %d nodeid %lu", m_sIP.c_str(), iIP, m_iPort, m_iNodeID);
}

void NodeInfo :: ParseNodeID()
{
    m_iPort = m_iNodeID & (0xffffffff);

    in_addr addr;
    addr.s_addr = m_iNodeID >> 32;
    
    m_sIP = std::string(inet_ntoa(addr));

    //PLImp("nodeid %lu ip %s ip %u port %d", m_iNodeID, m_sIP.c_str(), addr.s_addr, m_iPort);
}

/////////////////////////////////////////////////////////////

GroupSMInfo :: GroupSMInfo()
{
    iGroupIdx = -1;
    bIsUseMaster = false;
}

/////////////////////////////////////////////////////////////

Options :: Options()
{
    poLogStorage = nullptr;
    bSync = true;
    iSyncInterval = 0;
    poNetWork = nullptr;
    iUDPMaxSize = 4096;
    iGroupCount = 1;
    bUseMembership = false;
    pMembershipChangeCallback = nullptr;
    pMasterChangeCallback = nullptr;
    poBreakpoint = nullptr;
    bIsLargeValueMode = false;
    pLogFunc = nullptr;
    eLogLevel = LogLevel::LogLevel_None;
    bUseCheckpointReplayer = false;
    bUseBatchPropose = false;
    bOpenChangeValueBeforePropose = false;
}
    
}


