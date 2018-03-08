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
#include <typeinfo>
#include <inttypes.h>

namespace phxpaxos
{

//You can use your own network to make paxos communicate. :)

class Node;

class NetWork
{
public:
    NetWork();
    virtual ~NetWork() {}

    //Network must not send/recieve any message before paxoslib called this funtion.
    virtual void RunNetWork() = 0;

    //If paxoslib call this function, network need to stop receive any message.
    virtual void StopNetWork() = 0;

    virtual int SendMessageTCP(const int iGroupIdx, const std::string & sIp, const int iPort, const std::string & sMessage) = 0;

    virtual int SendMessageUDP(const int iGroupIdx, const std::string & sIp, const int iPort, const std::string & sMessage) = 0;

    //When receive a message, call this funtion.
    //This funtion is async, just enqueue an return.
    int OnReceiveMessage(const char * pcMessage, const int iMessageLen);

private:
    friend class Node;
    Node * m_poNode;
};
    
}
