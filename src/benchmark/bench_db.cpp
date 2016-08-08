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
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <typeinfo>
#include <inttypes.h>

using namespace phxpaxos;

void RandValue(const int iSize, string & sValue)
{
    int iRealSize = iSize;

    for (int i = 0; i < iRealSize; i++)
    {
        sValue += (rand() % 26 + 'a');
    }
}

const uint64_t GetSteadyClockMS()
{
    uint64_t llNow;
    struct timeval tv; 

    gettimeofday(&tv, NULL);

    llNow = tv.tv_sec;
    llNow *= 1000;
    llNow += tv.tv_usec / 1000; 

    return llNow;
}

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("%s <value size> <write times>\n", argv[0]);
        return 0;
    }

    string sLogStoragePath = "./bench_db_log_path";
    if (access(sLogStoragePath.c_str(), F_OK) == -1)
    {
        if (mkdir(sLogStoragePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
        {       
            printf("Create dir fail, path %s\n", sLogStoragePath.c_str());
            return -1;
        }       
    }

    int iValueSize = atoi(argv[1]);
    string sValue;
    RandValue(iValueSize, sValue);
    int wc = atoi(argv[2]);

    Database db;
    int ret = db.Init(sLogStoragePath, 0);
    if (ret != 0)
    {
        printf("db init fail, ret %d\n", ret);
        return ret;
    }

    uint64_t llInstanceID = 0;
    ret = db.GetMaxInstanceID(llInstanceID);
    if (ret != 0 && ret != 1)
    {
        printf("get db max instanceid fail\n");
        return -1;
    }

    WriteOptions wp;
    wp.bSync = true;

    uint64_t llBeginTimeMs = GetSteadyClockMS();
    for (int i = 0; i < wc; i++)
    {
        AcceptorStateData oState;
        oState.set_instanceid(llInstanceID);
        oState.set_promiseid(0);
        oState.set_promisenodeid(0);
        oState.set_acceptedid(0);
        oState.set_acceptednodeid(0);
        oState.set_acceptedvalue(sValue);
        oState.set_checksum(0);
        string sBuffer;
        bool bSucc = oState.SerializeToString(&sBuffer);
        if (!bSucc)
        {
            return -1;
        }

        ret = db.Put(wp, llInstanceID, sBuffer);
        if (ret != 0)
        {
            printf("put fail, instanceid %lu ret %d\n",
                    llInstanceID, ret);
            return ret;
        }
        llInstanceID++;
    }

    uint64_t llEndTimeMs = GetSteadyClockMS();
    int iRunTimeMs = llEndTimeMs - llBeginTimeMs;
    int qps = (uint64_t)wc * 1000 / iRunTimeMs;

    printf("qps %d\n", qps);

    return 0;
}

