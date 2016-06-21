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

#include "phxpaxos/network.h"
#include "phxpaxos/options.h"
#include <map>
#include "msg_transport.h"
#include "config_include.h"

namespace phxpaxos
{

class Communicate : public MsgTransport
{
public:
    Communicate(
            const Config * poConfig,
            const nodeid_t iMyNodeID, 
            const int iUDPMaxSize,
            NetWork * poNetwork);
    ~Communicate();

    int SendMessage(const nodeid_t iSendtoNodeID, const std::string & sMessage,
            const int iSendType = Message_SendType_UDP);

    int BroadcastMessage(const std::string & sMessage,
            const int iSendType = Message_SendType_UDP);

    int BroadcastMessageFollower(const std::string & sMessage,
            const int iSendType = Message_SendType_UDP);
    
    int BroadcastMessageTempNode(const std::string & sMessage,
            const int iSendType = Message_SendType_UDP);

public:
    void SetUDPMaxSize(const size_t iUDPMaxSize);

private:
    int Send(const nodeid_t iNodeID, const NodeInfo & tNodeInfo, const std::string & sMessage, const int iSendType);

private:
    Config * m_poConfig;
    NetWork * m_poNetwork;

    nodeid_t m_iMyNodeID;
    size_t m_iUDPMaxSize; 
};
    
}
