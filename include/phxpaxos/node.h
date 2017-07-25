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
#include <map>
#include <vector>

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

    virtual const uint64_t GetMinChosenInstanceID(const int iGroupIdx) = 0;

    virtual const nodeid_t GetMyNodeID() const = 0;

    //Batch propose.
    
    //Only set options::bUserBatchPropose as true can use this batch API.
    //Warning: BatchProposal will have same llInstanceID returned but different iBatchIndex.
    //Batch values's execute order in StateMachine is certain, the return value iBatchIndex
    //means the execute order index, start from 0.
    virtual int BatchPropose(const int iGroupIdx, const std::string & sValue, 
            uint64_t & llInstanceID, uint32_t & iBatchIndex) = 0;

    virtual int BatchPropose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, 
            uint32_t & iBatchIndex, SMCtx * poSMCtx) = 0;

    //PhxPaxos will batch proposal while waiting proposals count reach to BatchCount, 
    //or wait time reach to BatchDelayTimeMs.
    virtual void SetBatchCount(const int iGroupIdx, const int iBatchCount) = 0;

    virtual void SetBatchDelayTimeMs(const int iGroupIdx, const int iBatchDelayTimeMs) = 0;

    //State machine.
    
    //This function will add state machine to all group.
    virtual void AddStateMachine(StateMachine * poSM) = 0;
    
    virtual void AddStateMachine(const int iGroupIdx, StateMachine * poSM) = 0;

    //Timeout control.
    virtual void SetTimeoutMs(const int iTimeoutMs) = 0;

    //Checkpoint
    
    //Set the number you want to keep paxoslog's count.
    //We will only delete paxoslog before checkpoint instanceid.
    //If llHoldCount < 300, we will set it to 300. Not suggest too small holdcount.
    virtual void SetHoldPaxosLogCount(const uint64_t llHoldCount) = 0;

    //Replayer is to help sm make checkpoint.
    //Checkpoint replayer default is paused, if you not use this, ignord this function.
    //If sm use ExecuteForCheckpoint to make checkpoint, you need to run replayer(you can run in any time).
    
    //Pause checkpoint replayer.
    virtual void PauseCheckpointReplayer() = 0;

    //Continue to run replayer
    virtual void ContinueCheckpointReplayer() = 0;

    //Paxos log cleaner working for deleting paxoslog before checkpoint instanceid.
    //Paxos log cleaner default is pausing.
    
    //pause paxos log cleaner.
    virtual void PausePaxosLogCleaner() = 0;

    //Continue to run paxos log cleaner.
    virtual void ContinuePaxosLogCleaner() = 0;

    //Membership
    
    //Show now membership.
    virtual int ShowMembership(const int iGroupIdx, NodeInfoList & vecNodeInfoList) = 0;
    
    //Add a paxos node to membership.
    virtual int AddMember(const int iGroupIdx, const NodeInfo & oNode) = 0;

    //Remove a paxos node from membership.
    virtual int RemoveMember(const int iGroupIdx, const NodeInfo & oNode) = 0;

    //Change membership by one node to another node.
    virtual int ChangeMember(const int iGroupIdx, const NodeInfo & oFromNode, const NodeInfo & oToNode) = 0;

    //Master
    
    //Check who is master.
    virtual const NodeInfo GetMaster(const int iGroupIdx) = 0;

    //Check who is master and get version.
    virtual const NodeInfo GetMasterWithVersion(const int iGroupIdx, uint64_t & llVersion) = 0;
    
    //Check is i'm master.
    virtual const bool IsIMMaster(const int iGroupIdx) = 0;

    virtual int SetMasterLease(const int iGroupIdx, const int iLeaseTimeMs) = 0;

    virtual int DropMaster(const int iGroupIdx) = 0;

    //Qos

    //If many threads propose same group, that some threads will be on waiting status.
    //Set max hold threads, and we will reject some propose request to avoid to many threads be holded.
    //Reject propose request will get retcode(PaxosTryCommitRet_TooManyThreadWaiting_Reject), check on def.h.
    virtual void SetMaxHoldThreads(const int iGroupIdx, const int iMaxHoldThreads) = 0;

    //To avoid threads be holded too long time, we use this threshold to reject some propose to control thread's wait time.
    virtual void SetProposeWaitTimeThresholdMS(const int iGroupIdx, const int iWaitTimeThresholdMS) = 0;

    //write disk
    virtual void SetLogSync(const int iGroupIdx, const bool bLogSync) = 0;

    //Not suggest to use this function
    //pair: value,smid.
    //Because of BatchPropose, a InstanceID maybe include multi-value.
    virtual int GetInstanceValue(const int iGroupIdx, const uint64_t llInstanceID, 
            std::vector<std::pair<std::string, int> > & vecValues) = 0;

protected:
    friend class NetWork; 

    virtual int OnReceiveMessage(const char * pcMessage, const int iMessageLen) = 0;
};
    
}
