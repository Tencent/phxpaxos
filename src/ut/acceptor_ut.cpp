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


class AcceptorBuilder
{
public:
    AcceptorBuilder()
    {
        MakeConfig(&oMockLogStorage, poConfig);
        MakeCommunicate(&oMockNetWork, poConfig, poCommunicate);
        MakeInstance(&oMockLogStorage, poConfig, poCommunicate, poInstance);
        MakeAcceptor(&oMockLogStorage, poConfig, poCommunicate, poInstance, poAcceptor);

        poAcceptor->SetAsTestMode();

        BP->SetInstance(&oMockBreakpoint);
    }

    ~AcceptorBuilder()
    {
        delete poAcceptor;
        delete poCommunicate;
        delete poInstance;
        delete poConfig;
        BP->SetInstance(nullptr);
    }

    MockNetWork oMockNetWork;
    MockLogStorage oMockLogStorage;
    Config * poConfig;
    Communicate * poCommunicate;
    Instance * poInstance;
    Acceptor * poAcceptor;

    MockBreakpoint oMockBreakpoint;
};

TEST(Acceptor, OnPrepare_Promise)
{
    AcceptorBuilder ob;

    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(0));

    MockAcceptorBP & oAcceptorBP = ob.oMockBreakpoint.m_oMockAcceptorBP;
    EXPECT_CALL(oAcceptorBP, OnPreparePass()).Times(1);
    EXPECT_CALL(oAcceptorBP, OnPrepareReject()).Times(0);

    NodeInfo oMyNode = GetMyNode();

    ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot = BallotNumber(1, oMyNode.GetNodeID());

    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(2);
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosPrepare);

    ob.poAcceptor->OnPrepare(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(2, oMyNode.GetNodeID()));

    //second times
    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(0));

    EXPECT_CALL(oAcceptorBP, OnPreparePass()).Times(1);
    EXPECT_CALL(oAcceptorBP, OnPrepareReject()).Times(0);

    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(2);
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosPrepare);

    ob.poAcceptor->OnPrepare(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(2, oMyNode.GetNodeID()));

    //third times
    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(0));

    EXPECT_CALL(oAcceptorBP, OnPreparePass()).Times(1);
    EXPECT_CALL(oAcceptorBP, OnPrepareReject()).Times(0);

    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(3);
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosPrepare);

    ob.poAcceptor->OnPrepare(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(3, oMyNode.GetNodeID()));
}

TEST(Acceptor, OnPrepare_Reject)
{
    AcceptorBuilder ob;

    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).Times(0);

    MockAcceptorBP & oAcceptorBP = ob.oMockBreakpoint.m_oMockAcceptorBP;
    EXPECT_CALL(oAcceptorBP, OnPreparePass()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnPrepareReject()).Times(1);

    NodeInfo oMyNode = GetMyNode();

    ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot = BallotNumber(2, oMyNode.GetNodeID());

    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(1);
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosPrepare);

    ob.poAcceptor->OnPrepare(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(2, oMyNode.GetNodeID()));
}

TEST(Acceptor, OnPrepare_PersistFail)
{
    AcceptorBuilder ob;

    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(-1));

    MockAcceptorBP & oAcceptorBP = ob.oMockBreakpoint.m_oMockAcceptorBP;
    EXPECT_CALL(oAcceptorBP, OnPreparePass()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnPrepareReject()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnPreparePersistFail()).Times(1);

    NodeInfo oMyNode = GetMyNode();

    ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot = BallotNumber(2, oMyNode.GetNodeID());

    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(3);
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosPrepare);

    ob.poAcceptor->OnPrepare(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(3, oMyNode.GetNodeID()));
}

TEST(Acceptor, OnAccept_Pass)
{
    AcceptorBuilder ob;

    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(0));

    MockAcceptorBP & oAcceptorBP = ob.oMockBreakpoint.m_oMockAcceptorBP;
    EXPECT_CALL(oAcceptorBP, OnAcceptPass()).Times(1);
    EXPECT_CALL(oAcceptorBP, OnAcceptPersistFail()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnAcceptReject()).Times(0);

    NodeInfo oMyNode = GetMyNode();

    ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot = BallotNumber(10, oMyNode.GetNodeID());

    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(13);
    oPaxosMsg.set_value("hello paxos");
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosAccept);

    ob.poAcceptor->OnAccept(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(13, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oAcceptedBallot == BallotNumber(13, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_sAcceptedValue == "hello paxos");

    //second times
    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(0));

    EXPECT_CALL(oAcceptorBP, OnAcceptPass()).Times(1);
    EXPECT_CALL(oAcceptorBP, OnAcceptPersistFail()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnAcceptReject()).Times(0);

    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(14);
    oPaxosMsg.set_value("hello hello");
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosAccept);

    ob.poAcceptor->OnAccept(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(14, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oAcceptedBallot == BallotNumber(14, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_sAcceptedValue == "hello hello");

    //third times
    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(0));

    EXPECT_CALL(oAcceptorBP, OnAcceptPass()).Times(1);
    EXPECT_CALL(oAcceptorBP, OnAcceptPersistFail()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnAcceptReject()).Times(0);

    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(14);
    oPaxosMsg.set_value("yes we are");
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosAccept);

    ob.poAcceptor->OnAccept(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(14, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oAcceptedBallot == BallotNumber(14, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_sAcceptedValue == "yes we are");
}

TEST(Acceptor, OnAccept_Reject)
{
    AcceptorBuilder ob;

    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).Times(0);

    MockAcceptorBP & oAcceptorBP = ob.oMockBreakpoint.m_oMockAcceptorBP;
    EXPECT_CALL(oAcceptorBP, OnAcceptPass()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnAcceptPersistFail()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnAcceptReject()).Times(1);

    NodeInfo oMyNode = GetMyNode();

    ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot = BallotNumber(10, oMyNode.GetNodeID());

    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(9);
    oPaxosMsg.set_value("hello paxos");
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosAccept);

    ob.poAcceptor->OnAccept(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(10, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oAcceptedBallot == BallotNumber(0, 0));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_sAcceptedValue.size() == 0);
}

TEST(Acceptor, OnAccept_PersistFail)
{
    AcceptorBuilder ob;

    EXPECT_CALL(ob.oMockLogStorage, Put(_,_,_,_)).WillOnce(Return(-1));

    MockAcceptorBP & oAcceptorBP = ob.oMockBreakpoint.m_oMockAcceptorBP;
    EXPECT_CALL(oAcceptorBP, OnAcceptPass()).Times(0);
    EXPECT_CALL(oAcceptorBP, OnAcceptPersistFail()).Times(1);
    EXPECT_CALL(oAcceptorBP, OnAcceptReject()).Times(0);

    NodeInfo oMyNode = GetMyNode();

    ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot = BallotNumber(10, oMyNode.GetNodeID());

    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_instanceid(0);
    oPaxosMsg.set_nodeid(oMyNode.GetNodeID());
    oPaxosMsg.set_proposalid(11);
    oPaxosMsg.set_value("hello paxos");
    oPaxosMsg.set_msgtype(phxpaxos::MsgType_PaxosAccept);

    ob.poAcceptor->OnAccept(oPaxosMsg);

    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oPromiseBallot == BallotNumber(11, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_oAcceptedBallot == BallotNumber(11, oMyNode.GetNodeID()));
    EXPECT_TRUE(ob.poAcceptor->m_oAcceptorState.m_sAcceptedValue == "hello paxos");
}



