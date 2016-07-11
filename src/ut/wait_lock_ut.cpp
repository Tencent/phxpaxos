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
#include "comm_include.h"
#include <map>
#include "concurrent.h"
#include "gmock/gmock.h"

using namespace phxpaxos;
using namespace std;
using ::testing::_;
using ::testing::Return;

class LockTester : public Thread
{
public:
	LockTester(WaitLock * poLock)
		: m_poLock(poLock)
	{
	}

	~LockTester() { }

	void run()
	{
		int iLockUseTimeMs = 0;
		bool bHasLock = m_poLock->Lock(-1, iLockUseTimeMs);
		ASSERT_TRUE(bHasLock == true);
		int iSleepTime = 30;
		Time::MsSleep(iSleepTime);
		m_poLock->UnLock();

		if (iLockUseTimeMs > 0)
		{
			int iCut = abs(iLockUseTimeMs - iSleepTime);
			EXPECT_TRUE(iCut < 5);
		}
	}

private:
	WaitLock * m_poLock;
};

TEST(WaitLock, Lock)
{
	WaitLock oLock;

	LockTester * poTest1 = new LockTester(&oLock);
	poTest1->start();

	LockTester * poTest2 = new LockTester(&oLock);
	poTest2->start();

	poTest1->join();
	poTest2->join();

	delete poTest1;
	delete poTest2;
}

class LockTimeoutTester : public Thread
{
public:
	LockTimeoutTester(WaitLock * poLock)
		: m_poLock(poLock)
	{
	}

	~LockTimeoutTester() { }

	void run()
	{
		int iLockUseTimeMs = 0;
		int iLockTimeoutMs = 20;
		bool bHasLock = m_poLock->Lock(iLockTimeoutMs, iLockUseTimeMs);
		if (bHasLock)
		{
			EXPECT_TRUE(iLockUseTimeMs <= 1);
		}
		else
		{
			int iCut = abs(iLockTimeoutMs - iLockUseTimeMs);
			EXPECT_TRUE(iCut < 5);
			return;
		}
		
		int iSleepTime = 30;
		Time::MsSleep(iSleepTime);
		m_poLock->UnLock();
	}

private:
	WaitLock * m_poLock;
};

TEST(WaitLock, LockTimeout)
{
	WaitLock oLock;

	LockTimeoutTester * poTest1 = new LockTimeoutTester(&oLock);
	poTest1->start();

	LockTimeoutTester * poTest2 = new LockTimeoutTester(&oLock);
	poTest2->start();

	poTest1->join();
	poTest2->join();

	delete poTest1;
	delete poTest2;
}

class TooMuchLockWatingTester : public Thread
{
public:
	TooMuchLockWatingTester(WaitLock * poLock, int & iRejectCount)
		: m_poLock(poLock), m_iRejectCount(iRejectCount)
	{
	}

	~TooMuchLockWatingTester() { }

	void run()
	{
		int iLockUseTimeMs = 0;
		int iLockTimeoutMs = -1;
		bool bHasLock = m_poLock->Lock(iLockTimeoutMs, iLockUseTimeMs);
		if (bHasLock)
		{
		}
		else
		{
            if (iLockUseTimeMs > 0)
            {
                int iCut = abs(iLockTimeoutMs - iLockUseTimeMs);
                EXPECT_TRUE(iCut < 5);
                return;
            }
            else
            {
                m_iRejectCount++;
            }
		}
		
		int iSleepTime = 10;
		Time::MsSleep(iSleepTime);
		m_poLock->UnLock();
	}

private:
	WaitLock * m_poLock;
    int & m_iRejectCount;
};

TEST(WaitLock, TooMuchLockWating)
{
	WaitLock oLock;
    oLock.SetMaxWaitLogCount(5);
    int iRejectCount = 0;

    std::vector<TooMuchLockWatingTester *> vecTester;
    for (int i = 0; i < 10; i++)
    {
        auto poTester = new TooMuchLockWatingTester(&oLock, iRejectCount);
        vecTester.push_back(poTester);
        poTester->start();
		//Time::MsSleep(10);
    }

    for (auto & poTester : vecTester)
    {
        poTester->join();
        delete poTester;
    }

    EXPECT_TRUE(iRejectCount == 4);
}

