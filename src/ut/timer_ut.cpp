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
#include <math.h>
#include <map>
#include "gmock/gmock.h"

using namespace phxpaxos;
using namespace std;
using ::testing::_;
using ::testing::Return;

TEST(Timer, AddTimer)
{
	Timer oTimer;

	uint64_t llAbsTime = Time::GetSteadyClockMS() + 30;
	uint32_t iTimerID = 0;
	int iType = 321;
	oTimer.AddTimerWithType(llAbsTime, iType, iTimerID);

	//wait 30ms, and popout;
	Time::MsSleep(35);
	
	uint32_t iTakeTimerID = 0;
	int iTakeType = 0;
	bool bHasTimeout = oTimer.PopTimeout(iTakeTimerID, iTakeType);

	EXPECT_TRUE(bHasTimeout == true);
	EXPECT_TRUE(iTakeTimerID == iTimerID);
	EXPECT_TRUE(iTakeType == iType);
}

int PopTimeout(Timer & oTimer, std::map<uint32_t, uint64_t> & mapTimerIDtoAbsTime, bool & bPass)
{
	bool bHasTimeout = true;
	int iNextTimeout = 0;

	while(bHasTimeout)
	{
		uint32_t iTimerID = 0;
		int iType = 0;
		bHasTimeout = oTimer.PopTimeout(iTimerID, iType);

		if (bHasTimeout)
		{
			uint64_t llCut = mapTimerIDtoAbsTime[iTimerID] < Time::GetSteadyClockMS() ? 
				Time::GetSteadyClockMS() - mapTimerIDtoAbsTime[iTimerID]
				: mapTimerIDtoAbsTime[iTimerID] - Time::GetSteadyClockMS();
			
			if (llCut > 10)
			{
				bPass = false;
			}

			iNextTimeout = oTimer.GetNextTimeout();

			if (iNextTimeout != 0)
			{
				break;
			}
		}

	}

	return iNextTimeout;
}

TEST(Timer, PopTimer)
{
	Timer oTimer;

	std::map<uint32_t, uint64_t> mapTimerIDtoAbsTime;
	
	srand((unsigned int)time(NULL));

	int iTimerObjCount = 10;
	for (int i = 0; i < iTimerObjCount; i++)
	{
		uint64_t llAbsTime = Time::GetSteadyClockMS() + (OtherUtils::FastRand() % 500);
		uint32_t iTimerID = 0;

		oTimer.AddTimer(llAbsTime, iTimerID);
		mapTimerIDtoAbsTime[iTimerID] = llAbsTime;
	}

	bool bPass = true;
	int iNextTimeout = 0;
	while(iNextTimeout != -1)
	{
		iNextTimeout = PopTimeout(oTimer, mapTimerIDtoAbsTime, bPass);
		Time::MsSleep(iNextTimeout);
	}

	EXPECT_TRUE(bPass == true);
}


