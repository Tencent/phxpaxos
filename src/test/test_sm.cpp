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

#include "test_sm.h"

using namespace phxpaxos;
using namespace std;

namespace phxpaxos_test
{

TestSM :: TestSM()
{
}

bool TestSM :: Execute(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sPaxosValue, SMCtx * poSMCtx)
{
    m_vecExecuted.push_back(make_pair(llInstanceID, sPaxosValue));
    return true;
}

bool TestSM :: CheckExecuteValueCorrect(const std::vector<pair<uint64_t, std::string> > & vecOtherExecuted)
{
    if (vecOtherExecuted.size() != m_vecExecuted.size())
    {
        printf("size not same, other.size %zu excute.size %zu\n",
                vecOtherExecuted.size(), m_vecExecuted.size());
        return false;
    }

    if (vecOtherExecuted.size() == 0)
    {
        return true;
    }

    uint64_t llNowInstanceID = m_vecExecuted[0].first;
    for (auto & it : m_vecExecuted)
    {
        if (it.first < llNowInstanceID)
        {
            printf("instanceid not serial, actual %lu now %lu\n",
                    it.first, llNowInstanceID);
            return false;
        }

        llNowInstanceID = it.first;
    }

    for (size_t i = 0; i < m_vecExecuted.size(); i++)
    {
        if (m_vecExecuted[i].first != vecOtherExecuted[i].first
                || m_vecExecuted[i].second != vecOtherExecuted[i].second)
        {
            printf("not same %lu %s | %lu %s\n",
                    m_vecExecuted[i].first, m_vecExecuted[i].second.c_str(),
                    vecOtherExecuted[i].first, vecOtherExecuted[i].second.c_str());
            return false;
        }
    }

    return true;
}

}


