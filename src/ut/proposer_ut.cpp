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

#include <string>
#include "gmock/gmock.h"
#include "make_class.h"
#include "mock_class.h"

using namespace phxpaxos;
using namespace std;
using ::testing::_;
using ::testing::Return;


class ProposerBuilder
{
public:
    ProposerBuilder()
    {
        MakeConfig(&oMockLogStorage, poConfig);
        MakeCommunicate(&oMockNetWork, poConfig, poCommunicate);
        MakeInstance(&oMockLogStorage, poConfig, poCommunicate, poInstance);
        MakeProposer(poConfig, poCommunicate, poInstance, &oMockLearner, &oMockIOLoop, poProposer);

        poProposer->SetAsTestMode();

        BP->SetInstance(&oMockBreakpoint);
    }

    ~ProposerBuilder()
    {
        delete poProposer;
        delete poCommunicate;
        delete poInstance;
        delete poConfig;
        BP->SetInstance(nullptr);
    }

    MockNetWork oMockNetWork;
    MockLogStorage oMockLogStorage;
    MockIOLoop oMockIOLoop;
    MockLearner oMockLearner;

    Config * poConfig;
    Communicate * poCommunicate;
    Instance * poInstance;
    Proposer * poProposer;

    MockBreakpoint oMockBreakpoint;
};

TEST(Proposer, NewValue)
{
    ProposerBuilder ob;

    MockProposerBP & oProposerBP = ob.oMockBreakpoint.m_oMockProposerBP;

    ob.poProposer->m_oProposerState.m_llProposalID = 100;
    string sNewValue = "hello paxos";

    ob.poProposer->NewValue(sNewValue);

    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_llProposalID == 100);
    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == sNewValue);

    EXPECT_TRUE(ob.poProposer->m_bIsPreparing == true);

    //second call, new ballot

    ob.poProposer->m_bWasRejectBySomeone = true;
    ob.poProposer->NewValue(sNewValue);

    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_llProposalID == 101);
    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == sNewValue);

    //third call, skip prepare
    EXPECT_CALL(oProposerBP, NewProposalSkipPrepare()).Times(1);

    ob.poProposer->m_bCanSkipPrepare = true;
    ob.poProposer->NewValue(sNewValue);

    EXPECT_TRUE(ob.poProposer->m_bIsAccepting == true);
    EXPECT_TRUE(ob.poProposer->m_bIsPreparing == false);
}

TEST(Proposer, OnPrepareReply_Skip)
{
    ProposerBuilder ob;

    MockProposerBP & oProposerBP = ob.oMockBreakpoint.m_oMockProposerBP;

    ob.poProposer->m_oProposerState.m_llProposalID = 100;
    PaxosMsg oPaxosMsg;

    //first call
    EXPECT_CALL(oProposerBP, OnPrepareReplyButNotPreparing()).Times(1);
    ob.poProposer->m_bIsPreparing = false;
    ob.poProposer->OnPrepareReply(oPaxosMsg);

    //second call
    EXPECT_CALL(oProposerBP, OnPrepareReplyNotSameProposalIDMsg()).Times(1);
    oPaxosMsg.set_proposalid(102);
    ob.poProposer->m_bIsPreparing = true;
    ob.poProposer->OnPrepareReply(oPaxosMsg);
}

TEST(Proposer, OnPrepareReply_Pass)
{
    ProposerBuilder ob;

    MockProposerBP & oProposerBP = ob.oMockBreakpoint.m_oMockProposerBP;

    ob.poProposer->m_oProposerState.m_llProposalID = 100;
    ob.poProposer->m_bIsPreparing = true;
    ob.poProposer->m_oProposerState.m_sValue = "abc";
    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_proposalid(100);

    //first call
    oPaxosMsg.set_preacceptid(95);
    oPaxosMsg.set_preacceptnodeid(GetMyNode().GetNodeID());
    oPaxosMsg.set_nodeid(GetMyNode().GetNodeID());
    oPaxosMsg.set_value("hello paxos");
    ob.poProposer->OnPrepareReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == "hello paxos");

    //second call
    oPaxosMsg.set_rejectbypromiseid(101);
    oPaxosMsg.set_nodeid(2);
    ob.poProposer->OnPrepareReply(oPaxosMsg);

    //third call
    oPaxosMsg.set_rejectbypromiseid(0);
    oPaxosMsg.set_preacceptid(98);
    oPaxosMsg.set_preacceptnodeid(3);
    oPaxosMsg.set_nodeid(3);
    oPaxosMsg.set_value("hello world");

    EXPECT_CALL(oProposerBP, PreparePass(_)).Times(1);

    ob.poProposer->OnPrepareReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_bCanSkipPrepare == true);
    EXPECT_TRUE(ob.poProposer->m_bIsAccepting == true);
    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == "hello world");
}

TEST(Proposer, OnPrepareReply_Reject)
{
    ProposerBuilder ob;

    MockProposerBP & oProposerBP = ob.oMockBreakpoint.m_oMockProposerBP;

    ob.poProposer->m_oProposerState.m_llProposalID = 100;
    ob.poProposer->m_bIsPreparing = true;
    ob.poProposer->m_oProposerState.m_sValue = "abc";
    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_proposalid(100);

    //first call
    oPaxosMsg.set_preacceptid(95);
    oPaxosMsg.set_preacceptnodeid(GetMyNode().GetNodeID());
    oPaxosMsg.set_nodeid(GetMyNode().GetNodeID());
    oPaxosMsg.set_value("hello paxos");
    ob.poProposer->OnPrepareReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == "hello paxos");

    //second call
    oPaxosMsg.set_rejectbypromiseid(101);
    oPaxosMsg.set_nodeid(2);
    ob.poProposer->OnPrepareReply(oPaxosMsg);

    //third call
    oPaxosMsg.set_rejectbypromiseid(102);
    oPaxosMsg.set_nodeid(3);

    EXPECT_CALL(oProposerBP, PrepareNotPass()).Times(1);

    ob.poProposer->OnPrepareReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_bCanSkipPrepare == false);
    EXPECT_TRUE(ob.poProposer->m_bIsAccepting == false);
    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == "hello paxos");
}

TEST(Proposer, OnAcceptReply_Skip)
{
    ProposerBuilder ob;

    MockProposerBP & oProposerBP = ob.oMockBreakpoint.m_oMockProposerBP;

    ob.poProposer->m_oProposerState.m_llProposalID = 100;
    PaxosMsg oPaxosMsg;

    //first call
    EXPECT_CALL(oProposerBP, OnAcceptReplyButNotAccepting()).Times(1);
    ob.poProposer->m_bIsAccepting = false;
    ob.poProposer->OnAcceptReply(oPaxosMsg);

    //second call
    EXPECT_CALL(oProposerBP, OnAcceptReplyNotSameProposalIDMsg()).Times(1);
    oPaxosMsg.set_proposalid(102);
    ob.poProposer->m_bIsAccepting = true;
    ob.poProposer->OnAcceptReply(oPaxosMsg);
}

TEST(Proposer, OnAcceptReply_Reject)
{
    ProposerBuilder ob;

    MockProposerBP & oProposerBP = ob.oMockBreakpoint.m_oMockProposerBP;

    ob.poProposer->m_oProposerState.m_llProposalID = 100;
    ob.poProposer->m_bIsAccepting = true;
    ob.poProposer->m_oProposerState.m_sValue = "abc";
    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_proposalid(100);

    //first call
    oPaxosMsg.set_rejectbypromiseid(101);
    oPaxosMsg.set_nodeid(2);
    ob.poProposer->OnAcceptReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_bWasRejectBySomeone == true);

    //second call
    oPaxosMsg.set_rejectbypromiseid(0);
    oPaxosMsg.set_nodeid(GetMyNode().GetNodeID());
    ob.poProposer->OnAcceptReply(oPaxosMsg);

    //third call
    oPaxosMsg.set_rejectbypromiseid(102);
    oPaxosMsg.set_nodeid(3);

    EXPECT_CALL(oProposerBP, AcceptNotPass()).Times(1);

    ob.poProposer->OnAcceptReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_bCanSkipPrepare == false);
    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == "abc");
}

TEST(Proposer, OnAcceptReply_Pass)
{
    ProposerBuilder ob;

    MockProposerBP & oProposerBP = ob.oMockBreakpoint.m_oMockProposerBP;

    ob.poProposer->m_oProposerState.m_llProposalID = 100;
    ob.poProposer->m_bIsAccepting = true;
    ob.poProposer->m_oProposerState.m_sValue = "abc";
    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_proposalid(100);

    //first call
    oPaxosMsg.set_rejectbypromiseid(0);
    oPaxosMsg.set_nodeid(2);
    ob.poProposer->OnAcceptReply(oPaxosMsg);

    //second call
    oPaxosMsg.set_rejectbypromiseid(101);
    oPaxosMsg.set_nodeid(GetMyNode().GetNodeID());
    ob.poProposer->OnAcceptReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_bWasRejectBySomeone == true);

    //third call
    oPaxosMsg.set_rejectbypromiseid(0);
    oPaxosMsg.set_nodeid(3);

    EXPECT_CALL(oProposerBP, AcceptPass(_)).Times(1);
    EXPECT_CALL(ob.oMockLearner, ProposerSendSuccess(_,100)).Times(1);

    ob.poProposer->OnAcceptReply(oPaxosMsg);

    EXPECT_TRUE(ob.poProposer->m_bIsAccepting == false);
    EXPECT_TRUE(ob.poProposer->m_bIsPreparing == false);
    EXPECT_TRUE(ob.poProposer->m_oProposerState.m_sValue == "abc");
}


