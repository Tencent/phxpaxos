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
#include "gmock/gmock.h"
#include "phxpaxos/network.h"
#include "phxpaxos/storage.h"
#include "phxpaxos/breakpoint.h"
#include "ioloop.h"
#include "learner.h"
#include "acceptor.h"

class MockNetWork : public phxpaxos::NetWork
{
public:
    MOCK_METHOD0(RunNetWork, void());
    MOCK_METHOD0(StopNetWork, void());
    MOCK_METHOD3(SendMessageTCP, int(const std::string & sIp, const int iPort, const std::string & sMessage));
    MOCK_METHOD3(SendMessageUDP, int(const std::string & sIp, const int iPort, const std::string & sMessage));
};

class MockLogStorage : public phxpaxos::LogStorage
{
public:
    const std::string GetLogStorageDirPath(const int iGroupIdx) { return ""; }
    MOCK_METHOD3(Get, int(const int iGroupIdx, const uint64_t llInstanceID, std::string & sValue));
    MOCK_METHOD4(Put, int(const phxpaxos::WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID, const std::string & sValue));
    MOCK_METHOD3(Del, int(const phxpaxos::WriteOptions & oWriteOptions, int iGroupIdx, const uint64_t llInstanceID));
    MOCK_METHOD2(GetMaxInstanceID, int(const int iGroupIdx, uint64_t & llInstanceID));
    MOCK_METHOD3(SetMinChosenInstanceID, int(const phxpaxos::WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llMinInstanceID));
    MOCK_METHOD2(GetMinChosenInstanceID, int(const int iGroupIdx, uint64_t & llMinInstanceID));
    int ClearAllLog(const int iGroupIdx) { return 0; }
    MOCK_METHOD3(SetSystemVariables, int(const phxpaxos::WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer));
    MOCK_METHOD2(GetSystemVariables, int(const int iGroupIdx, std::string & sBuffer));
    MOCK_METHOD3(SetMasterVariables, int(const phxpaxos::WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer));
    MOCK_METHOD2(GetMasterVariables, int(const int iGroupIdx, std::string & sBuffer));
};

class MockIOLoop : public phxpaxos::IOLoop
{
public:
    MockIOLoop() : IOLoop(nullptr, nullptr) { }
    //MOCK_METHOD3(AddTimer, bool(const int iTimeout, const int iType, uint32_t & iTimerID));
    //MOCK_METHOD1(RemoveTimer, void(uint32_t & iTimerID));
};

class MockLearner : public phxpaxos::Learner
{
public:
    MockLearner() : Learner(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) { }
    MOCK_METHOD2(ProposerSendSuccess, void(const uint64_t llLearnInstanceID, const uint64_t llProposalID));
};

////////////////////////////////////////////////////

class MockAcceptorBP : public phxpaxos::AcceptorBP
{
public:
    MOCK_METHOD0(OnPreparePass, void());
    MOCK_METHOD0(OnPreparePersistFail, void());
    MOCK_METHOD0(OnPrepareReject, void());
    MOCK_METHOD0(OnAcceptPass, void());
    MOCK_METHOD0(OnAcceptPersistFail, void());
    MOCK_METHOD0(OnAcceptReject, void());
};

class MockProposerBP : public phxpaxos::ProposerBP
{
public:
    MOCK_METHOD0(NewProposalSkipPrepare, void()); 
    MOCK_METHOD0(OnPrepareReplyButNotPreparing, void()); 
    MOCK_METHOD0(OnPrepareReplyNotSameProposalIDMsg, void()); 
    MOCK_METHOD1(PreparePass, void(const int iUseTimeMs)); 
    MOCK_METHOD0(PrepareNotPass, void()); 
    MOCK_METHOD0(OnAcceptReplyButNotAccepting, void()); 
    MOCK_METHOD0(OnAcceptReplyNotSameProposalIDMsg, void()); 
    MOCK_METHOD1(AcceptPass, void(const int iUseTimeMs)); 
    MOCK_METHOD0(AcceptNotPass, void()); 
};

class MockBreakpoint : public phxpaxos::Breakpoint
{
public:
    phxpaxos::AcceptorBP * GetAcceptorBP() { return &m_oMockAcceptorBP; }
    phxpaxos::ProposerBP * GetProposerBP() { return &m_oMockProposerBP; }

    MockAcceptorBP m_oMockAcceptorBP;
    MockProposerBP m_oMockProposerBP;
};

