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
#include "db.h"
#include "gmock/gmock.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace phxpaxos;
using namespace std;
using ::testing::_;
using ::testing::Return;

int MakeLogStoragePath(std::string & sLogStoragePath)
{
    sLogStoragePath = "./ut_test_db_path/";

    if (access(sLogStoragePath.c_str(), F_OK) != -1)
    {
        if (FileUtils :: DeleteDir(sLogStoragePath) != 0)
        {
            printf("Delete exist logstorage dir fail\n");
            return -1;
        }
    }

    if (mkdir(sLogStoragePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
    {       
        printf("Create dir fail, path %s\n", sLogStoragePath.c_str());
        return -1;
    }       

    return 0;
}

int InitDB(const int iGroupCount, MultiDatabase & oDB)
{
	string sDBPath;
    int ret = MakeLogStoragePath(sDBPath);
    if (ret != 0)
    {
        return ret;
    }

	ret = oDB.Init(sDBPath, iGroupCount);
	if (ret != 0)
	{
		return ret;
	}
	
	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ret = oDB.ClearAllLog(iGroupIdx);
		if (ret != 0)
		{
			return ret;
		}
	}
	
	return 0;
}

TEST(MultiDatabase, ClearAllLog)
{
	int iGroupCount = 2;
	MultiDatabase oDB;
	ASSERT_TRUE(InitDB(iGroupCount, oDB) == 0);

	const uint64_t llInstanceID = 3;
	std::string sValue = "hello paxos";

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		WriteOptions oWriteOptions;
		oWriteOptions.bSync = true;

		ASSERT_TRUE(oDB.Put(oWriteOptions, iGroupIdx, llInstanceID, sValue) == 0);
	}

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.ClearAllLog(iGroupIdx) == 0);
	}

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		const uint64_t llInstanceID = 3;
		std::string sGetValue;

		ASSERT_TRUE(oDB.Get(iGroupIdx, llInstanceID, sGetValue) == 1);
	}
}

TEST(MultiDatabase, PUT_GET)
{
	int iGroupCount = 2;
	MultiDatabase oDB;
	ASSERT_TRUE(InitDB(iGroupCount, oDB) == 0);

	const uint64_t llInstanceID = 3;
	std::string sValue = "hello paxos";
	
	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		std::string sGetValue;
		ASSERT_TRUE(oDB.Get(iGroupIdx, llInstanceID, sGetValue) == 1);
	}

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		WriteOptions oWriteOptions;
		oWriteOptions.bSync = true;

		ASSERT_TRUE(oDB.Put(oWriteOptions, iGroupIdx, llInstanceID, sValue) == 0);
	}

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		std::string sGetValue;

		ASSERT_TRUE(oDB.Get(iGroupIdx, llInstanceID, sGetValue) == 0);
	}
}


TEST(MultiDatabase, Del)
{
	int iGroupCount = 2;
	MultiDatabase oDB;
	ASSERT_TRUE(InitDB(iGroupCount, oDB) == 0);

	const uint64_t llInstanceID = 3;
	std::string sValue = "hello paxos";
	WriteOptions oWriteOptions;
	oWriteOptions.bSync = true;

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.Put(oWriteOptions, iGroupIdx, llInstanceID, sValue) == 0);
	}

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.Del(oWriteOptions, iGroupIdx, llInstanceID) == 0);
	}

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.Del(oWriteOptions, iGroupIdx, llInstanceID) == 0);
	}
	
	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		std::string sGetValue;
		ASSERT_TRUE(oDB.Get(iGroupIdx, llInstanceID, sGetValue) == 1);
	}
}

TEST(MultiDatabase, GetMaxInstanceID)
{
	int iGroupCount = 2;
	MultiDatabase oDB;
	ASSERT_TRUE(InitDB(iGroupCount, oDB) == 0);

	const uint64_t llInstanceID = 3;
	std::string sValue = "hello paxos";
	WriteOptions oWriteOptions;
	oWriteOptions.bSync = true;

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.Put(oWriteOptions, iGroupIdx, llInstanceID + 1, sValue) == 0);
		ASSERT_TRUE(oDB.Put(oWriteOptions, iGroupIdx, llInstanceID + 3, sValue) == 0);

		uint64_t llMaxInstanceID = 0;
		ASSERT_TRUE(oDB.GetMaxInstanceID(iGroupIdx, llMaxInstanceID) == 0);
		EXPECT_TRUE(llMaxInstanceID == llInstanceID + 3);
	}
}

TEST(MultiDatabase, Set_Get_MinChosenInstanceID)
{
	int iGroupCount = 2;
	MultiDatabase oDB;
	ASSERT_TRUE(InitDB(iGroupCount, oDB) == 0);

	const uint64_t llMinChosenInstanceID = 102342342342lu;
	WriteOptions oWriteOptions;
	oWriteOptions.bSync = true;

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.SetMinChosenInstanceID(oWriteOptions, iGroupIdx, llMinChosenInstanceID) == 0);
		
		uint64_t llGetMinChosenInstanceID = 0;
		ASSERT_TRUE(oDB.GetMinChosenInstanceID(iGroupIdx, llGetMinChosenInstanceID) == 0);

		EXPECT_TRUE(llMinChosenInstanceID == llGetMinChosenInstanceID);
	}
}

TEST(MultiDatabase, Set_Get_SystemVariables)
{
	int iGroupCount = 2;
	MultiDatabase oDB;
	ASSERT_TRUE(InitDB(iGroupCount, oDB) == 0);

	string sBuffer = "234iskfj23io4uskfjalfj238j423j4l2k3j4lasklfjaslkfj28934j2l3j4lajflsjdlf";
	WriteOptions oWriteOptions;
	oWriteOptions.bSync = true;

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.SetSystemVariables(oWriteOptions, iGroupIdx, sBuffer) == 0);
		
		string sGetBuffer;
		ASSERT_TRUE(oDB.GetSystemVariables(iGroupIdx, sGetBuffer) == 0);

		EXPECT_TRUE(sBuffer == sGetBuffer);
	}
}

TEST(MultiDatabase, Set_Get_MasterVariables)
{
	int iGroupCount = 2;
	MultiDatabase oDB;
	ASSERT_TRUE(InitDB(iGroupCount, oDB) == 0);

	string sBuffer = "234fj238j423j4l2k3j4lasklfjaslkfj28934j2l3j4lajflsjdlf";
	WriteOptions oWriteOptions;
	oWriteOptions.bSync = true;

	for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
	{
		ASSERT_TRUE(oDB.SetMasterVariables(oWriteOptions, iGroupIdx, sBuffer) == 0);
		
		string sGetBuffer;
		ASSERT_TRUE(oDB.GetMasterVariables(iGroupIdx, sGetBuffer) == 0);

		EXPECT_TRUE(sBuffer == sGetBuffer);
	}
}


