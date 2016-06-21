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

#include "phxpaxos/options.h"

namespace phxpaxos
{

enum Message_SendType
{
    Message_SendType_UDP = 0,
    Message_SendType_TCP = 1,
};

class MsgTransport
{
public:
    virtual ~MsgTransport() {}

    virtual int SendMessage(const nodeid_t iSendtoNodeID, const std::string & sBuffer, 
            const int iSendType = Message_SendType_UDP) = 0;

    virtual int BroadcastMessage(const std::string & sBuffer, 
            const int iSendType = Message_SendType_UDP) = 0;
    
    virtual int BroadcastMessageFollower(const std::string & sBuffer, 
            const int iSendType = Message_SendType_UDP) = 0;
    
    virtual int BroadcastMessageTempNode(const std::string & sBuffer, 
            const int iSendType = Message_SendType_UDP) = 0;
};
    
}
