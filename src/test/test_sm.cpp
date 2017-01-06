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
#include "commdef.h"
#include "utils_include.h"

using namespace phxpaxos;
using namespace std;

namespace phxpaxos_test
{

TestSM :: TestSM()
{
    m_iLastValueChecksum = 0;
}

bool TestSM :: Execute(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sPaxosValue, SMCtx * poSMCtx)
{
    uint32_t iOtherLastChecksum = 0;
    string sBodyValue;
    UnPackTestValue(sPaxosValue, sBodyValue, iOtherLastChecksum);
    NLDebug("instanceid %lu other %u my %u", llInstanceID, iOtherLastChecksum, m_iLastValueChecksum);

    if (iOtherLastChecksum != 0 && iOtherLastChecksum != m_iLastValueChecksum)
    {
        printf("instanceid %lu other last check sum %u my last check sum %u\n",
                llInstanceID, iOtherLastChecksum, m_iLastValueChecksum);
        assert(iOtherLastChecksum == m_iLastValueChecksum);
    }

    m_vecExecuted.push_back(make_pair(llInstanceID, sBodyValue));

    m_iLastValueChecksum = crc32(m_iLastValueChecksum, (const uint8_t*)sBodyValue.data(), sBodyValue.size(), CRC32SKIP);
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

////////////////////////////

void TestSM :: BeforePropose(const int iGroupIdx, std::string & sValue)
{
    PackTestValueWithChecksum(sValue, m_iLastValueChecksum);
    NLDebug("after value size %zu", sValue.size());
}

const bool TestSM :: NeedCallBeforePropose()
{
    return rand() % 3 == 0 ? true : false;
}

std::string TestSM :: PackTestValue(const std::string & sValue)
{
    return "0" + sValue;
}

void TestSM :: PackTestValueWithChecksum(std::string & sValue, const uint32_t iLastChecksum)
{
    char sChecksum[sizeof(uint32_t)];
    memcpy(sChecksum, &iLastChecksum, sizeof(uint32_t));
    sValue = "1" + string(sChecksum, sizeof(uint32_t)) + sValue.substr(1, sValue.size());
}

void TestSM :: UnPackTestValue(const std::string & sValue, std::string & sBodyValue, uint32_t & iLastChecksum)
{
    if (sValue[0] == '0')
    {
        iLastChecksum = 0;
        sBodyValue = sValue.substr(1, sValue.size());
    }
    else
    {
        memcpy(&iLastChecksum, sValue.data() + 1, sizeof(uint32_t));
        sBodyValue = sValue.substr(1 + sizeof(uint32_t), sValue.size());
    }
}

}


