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

#include "phxpaxos/sm.h"
#include "phxpaxos/options.h"
#include <typeinfo>
#include <inttypes.h>

namespace phxpaxos
{

class NetWork;

//All the funciton in class Node is thread safe!

class Node
{
public:
    Node() { }
    virtual ~Node() { }

    //If you want to end paxos, just delete poNode.
    //But if you use your own network, poNode can be deleted after your own network stop recieving messages. 
    static int RunNode(const Options & oOptions, Node *& poNode);

    //Base function.
    virtual int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID) = 0;

    virtual int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, SMCtx * poSMCtx) = 0;

    virtual const uint64_t GetNowInstanceID(const int iGroupIdx) = 0;

    virtual const nodeid_t GetMyNodeID() const = 0;

    //State machine.
    
    //This function will add state machine to all group.
    virtual void AddStateMachine(StateMachine * poSM) = 0;
    
    virtual void AddStateMachine(const int iGroupIdx, StateMachine * poSM) = 0;

    //Timeout control.
    virtual void SetTimeoutMs(const int iTimeoutMs) = 0;

    //Checkpoint
    
    //Set the number you want to keep paxoslog's count.
    //We will only delete paxoslog before checkpoint instanceid.
    virtual void SetHoldPaxosLogCount(const uint64_t llHoldCount) = 0;

    //Replayer is to help sm make checkpoint.
    //Checkpoint replayer default is paused, if you not use this, ignord this function.
    //If sm use ExecuteForCheckpoint to make checkpoint, you need to run replayer(you can run in any time).
    
    //Pause checkpoint replayer.
    virtual void PauseCheckpointReplayer() = 0;

    //Continue to run replayer
    virtual void ContinueCheckpointReplayer() = 0;

    //Paxos log cleaner working for deleting paxoslog before checkpoint instanceid.
    //Paxos log cleaner default is running.
    
    //pause paxos log cleaner.
    virtual void PausePaxosLogCleaner() = 0;

    //Continue to run paxos log cleaner.
    virtual void ContinuePaxosLogCleaner() = 0;

    //Master
    
    //Check who is master.
    virtual const NodeInfo GetMaster(const int iGroupIdx) = 0;
    
    //Check is i'm master.
    virtual const bool IsIMMaster(const int iGroupIdx) = 0;

    virtual int SetMasterLease(const int iGroupIdx, const int iLeaseTimeMs) = 0;

    virtual int DropMaster(const int iGroupIdx) = 0;

protected:
    friend class NetWork; 

    virtual int OnReceiveMessage(const char * pcMessage, const int iMessageLen) = 0;
};
    
}
