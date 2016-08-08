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

#include "db.h"
#include <stdio.h>
#include <string>

using namespace std;
using namespace phxpaxos;

void DelInstance(const int iGroupIdx, MultiDatabase & oLogStorage, uint64_t llInstanceID)
{
    string sValue;
    int ret = oLogStorage.Get(iGroupIdx, llInstanceID, sValue);
    if (ret != 0)
    {
        printf("get this instance %lu fail, do you want to delete it? (y/n)\n", llInstanceID);
    }
    else
    {
        printf("this instance %lu value size is %zu, do you want to delete it? (y/n)\n", 
                llInstanceID, sValue.size());
    }

    string c = "n";
    cin >> c;

    if (c == "y")
    {
        WriteOptions oWriteOptions;
        oWriteOptions.bSync = true;
        ret = oLogStorage.ForceDel(oWriteOptions, iGroupIdx, llInstanceID);
        if (ret != 0)
        {
            printf("delete instance %lu fail\n", llInstanceID);
        }
        else
        {
            printf("delete instance %lu ok\n", llInstanceID);
        }
    }
}

void GetInstance(const int iGroupIdx, MultiDatabase & oLogStorage, uint64_t llInstanceID)
{
    string sValue;
    int ret = oLogStorage.Get(iGroupIdx, llInstanceID, sValue);
    if (ret != 0)
    {
        printf("get this instance %lu fail\n", llInstanceID);
        return;
    }

    AcceptorStateData oState;
    bool bSucc = oState.ParseFromArray(sValue.data(), sValue.size());
    if (!bSucc)
    {
        printf("parse from array fail, buffer size %zu\n", sValue.size());
        return;
    }

    printf("------------------------------------------------------\n");
    printf("instanceid %llu\n", oState.instanceid());
    printf("promiseid %llu\n", oState.promiseid());
    printf("promisenodid %llu\n", oState.promisenodeid());
    printf("acceptedid %llu\n", oState.acceptedid());
    printf("acceptednodeid %llu\n", oState.acceptednodeid());
    printf("acceptedvaluesize %zu\n", oState.acceptedvalue().size());
    printf("checksum %u\n", oState.checksum());
}

void GetMaxInstanceID(const int iGroupIdx, MultiDatabase & oLogStorage)
{
    uint64_t llInstanceID = 0;
    int ret = oLogStorage.GetMaxInstanceID(iGroupIdx, llInstanceID);
    if (ret != 0)
    {
        printf("get max instanceid fail, ret %d\n", ret);
        return;
    }

    printf("max instanceid %lu\n", llInstanceID);
}

int main(int argc, char ** argv)
{
    if (argc < 4)
    {
        printf("%s <paxos log dir> <group idx> <getmaxi>\n", argv[0]);
        printf("%s <paxos log dir> <group idx> <del> <instanceid>\n", argv[0]);
        printf("%s <paxos log dir> <group idx> <get> <instanceid>\n", argv[0]);
        return -1;
    }

    LOGGER->InitLogger(LogLevel::LogLevel_Verbose);

    string sPaxosLogPath = argv[1];
    int iGroupIdx = atoi(argv[2]);

    if (sPaxosLogPath.size() == 0)
    {
        printf("paxos log path not valid, path %s\n", sPaxosLogPath.c_str());
        return -1;
    }

    MultiDatabase oDefaultLogStorage;
    int ret = oDefaultLogStorage.Init(sPaxosLogPath, iGroupIdx + 1);
    if (ret != 0)
    {
        printf("init log storage fail, ret %d\n", ret);
        return ret;
    }

    string sOP = string(argv[3]);
    uint64_t llInstanceID = 0;
    if (argc > 4)
    {
        llInstanceID = strtoull(argv[4], NULL, 10);
    }

    if (sOP == "getmaxi")
    {
        GetMaxInstanceID(iGroupIdx, oDefaultLogStorage);
    }
    else if (sOP == "del")
    {
        DelInstance(iGroupIdx, oDefaultLogStorage, llInstanceID);
    } 
    else if (sOP == "get")
    {
        GetInstance(iGroupIdx, oDefaultLogStorage, llInstanceID);
    }

    return 0;
}


