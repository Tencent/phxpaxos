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
#include <set>
#include "commdef.h"

namespace phxpaxos
{

class Config;
class PaxosLog;

class MsgCounter
{
public:
    MsgCounter(const Config * poConfig);
    ~MsgCounter();

    void AddReceive(const nodeid_t iNodeID);
    void AddReject(const nodeid_t iNodeID);
    void AddPromiseOrAccept(const nodeid_t iNodeID);

    bool IsPassedOnThisRound();
    bool IsRejectedOnThisRound();
    bool IsAllReceiveOnThisRound();

    void StartNewRound();

public:
    Config * m_poConfig;

    std::set<nodeid_t> m_setReceiveMsgNodeID;
    std::set<nodeid_t> m_setRejectMsgNodeID;
    std::set<nodeid_t> m_setPromiseOrAcceptMsgNodeID;
};
    
}
