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
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <map>

namespace phxpaxos_test
{

class TestSM : public phxpaxos::StateMachine
{
public:
    TestSM();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, phxpaxos::SMCtx * poSMCtx);

    const int SMID() const { return 1; }

    bool CheckExecuteValueCorrect(const std::vector<std::pair<uint64_t, std::string> > & vecOtherExecuted);

    void BeforePropose(const int iGroupIdx, std::string & sValue);

    const bool NeedCallBeforePropose();

public:
    static std::string PackTestValue(const std::string & sValue);

    static void PackTestValueWithChecksum(std::string & sValue, const uint32_t iLastChecksum);

    static void UnPackTestValue(const std::string & sValue, std::string & sBodyValue, uint32_t & iLastChecksum);

public:
    std::vector<std::pair<uint64_t, std::string> > m_vecExecuted;

private:
    uint32_t m_iLastValueChecksum;
};
    
}
