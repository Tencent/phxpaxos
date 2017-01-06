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

#include "make_class.h"
#include "phxpaxos/options.h"
#include "mock_class.h"

using ::testing::_;
using ::testing::Return;

namespace phxpaxos
{

NodeInfo GetMyNode()
{
    string sIP = "127.0.0.1";
    int iPort = 11111;

    NodeInfo oMyNode(sIP, iPort);
    return oMyNode;
}

void MakeConfig(MockLogStorage * poMockLogStorage, Config *& poConfig)
{
    string sIP = "127.0.0.1";
    int iPort = 11111;

    NodeInfo oMyNode(sIP, iPort);
    NodeInfoList vecNodeInfoList;

    for (int i = 0; i < 3; i++)
    {
        vecNodeInfoList.push_back(NodeInfo(sIP, iPort));
        iPort++;
    }

    FollowerNodeInfoList vecFollowerNodeInfoList;

    int iMyGroupIdx = 0;
    int iGroupCount = 1;

    poConfig = nullptr;
    poConfig = new Config(poMockLogStorage, true, 0, false, oMyNode, vecNodeInfoList, vecFollowerNodeInfoList, iMyGroupIdx, iGroupCount, nullptr);
    assert(poConfig != nullptr);

    EXPECT_CALL(*poMockLogStorage, GetSystemVariables(_,_)).Times(1).WillOnce(Return(1));

    poConfig->Init();
}

void MakeCommunicate(MockNetWork * poMockNetWork, Config * poConfig, Communicate *& poCommunicate)
{
    poCommunicate = nullptr;
    poCommunicate = new Communicate(poConfig, poConfig->GetMyNodeID(), 2048, poMockNetWork);
    assert(poCommunicate != nullptr);
}

void MakeInstance(MockLogStorage * poMockLogStorage, Config * poConfig, Communicate * poCommunicate, Instance *& poInstance)
{
    poInstance = nullptr;
    Options oOptions;
    poInstance = new Instance(poConfig, poMockLogStorage, poCommunicate, oOptions);
    assert(poInstance != nullptr);
}

void MakeAcceptor(MockLogStorage * poMockLogStorage, Config * poConfig, Communicate * poCommunicate, Instance * poInstance, Acceptor *& poAcceptor)
{
    poAcceptor = nullptr;
    poAcceptor = new Acceptor(poConfig, poCommunicate, poInstance, poMockLogStorage);
    assert(poAcceptor != nullptr);
}

void MakeProposer(Config * poConfig, Communicate * poCommunicate, Instance * poInstance, Learner * poLearner, IOLoop * poIOLoop, Proposer *& poProposer)
{
    poProposer = nullptr;
    poProposer = new Proposer(poConfig, poCommunicate, poInstance, poLearner, poIOLoop);
    assert(poProposer != nullptr);
}

}




