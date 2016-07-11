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

#include "msg_counter.h"
#include <math.h>
#include "config_include.h"

namespace phxpaxos
{

MsgCounter :: MsgCounter(const Config * poConfig)
{
    m_poConfig = (Config *)poConfig;
    StartNewRound();
}

MsgCounter :: ~MsgCounter()
{
}

void MsgCounter :: StartNewRound()
{
    m_setReceiveMsgNodeID.clear();
    m_setRejectMsgNodeID.clear();
    m_setPromiseOrAcceptMsgNodeID.clear();
}

void MsgCounter :: AddReceive(const nodeid_t iNodeID)
{
    if (m_setReceiveMsgNodeID.find(iNodeID) == m_setReceiveMsgNodeID.end())
    {
        m_setReceiveMsgNodeID.insert(iNodeID);
    }
}

void MsgCounter :: AddReject(const nodeid_t iNodeID)
{
    if (m_setRejectMsgNodeID.find(iNodeID) == m_setRejectMsgNodeID.end())
    {
        m_setRejectMsgNodeID.insert(iNodeID);
    }
}

void MsgCounter :: AddPromiseOrAccept(const nodeid_t iNodeID)
{
    if (m_setPromiseOrAcceptMsgNodeID.find(iNodeID) == m_setPromiseOrAcceptMsgNodeID.end())
    {
        m_setPromiseOrAcceptMsgNodeID.insert(iNodeID);
    }
}

bool MsgCounter :: IsPassedOnThisRound()
{
    return (int)m_setPromiseOrAcceptMsgNodeID.size() >= m_poConfig->GetMajorityCount();
}

bool MsgCounter :: IsRejectedOnThisRound()
{
    return (int)m_setRejectMsgNodeID.size() >= m_poConfig->GetMajorityCount();
}

bool MsgCounter :: IsAllReceiveOnThisRound()
{
    return (int)m_setReceiveMsgNodeID.size() == m_poConfig->GetNodeCount();
}

}


