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

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <typeinfo>
#include <inttypes.h>

using namespace std;

void RandValue(const int iSize, string & sValue)
{
    sValue = "";
    int iRealSize = rand() % iSize + iSize / 2;

    for (int i = 0; i < iRealSize; i++)
    {
        sValue += (rand() % 26 + 'a');
    }
}

const uint64_t GetTimestampMS()
{
    uint64_t llNow;
    struct timeval tv; 

    gettimeofday(&tv, NULL);

    llNow = tv.tv_sec;
    llNow *= 1000;
    llNow += tv.tv_usec / 1000; 

    return llNow;
}

void benchfsync(const std::string & sFilePath)
{
    string sValue;
    RandValue(100, sValue);

    int fd = open(sFilePath.c_str(), O_CREAT | O_RDWR | O_APPEND, S_IWRITE | S_IREAD);
    if (fd == -1)
    {
        printf("open file fail %s\n", sFilePath.c_str());
        return;
    }

    uint64_t llBeginTimeMs = GetTimestampMS();

    const int iWriteCount = 100;
    for (int i = 0; i < iWriteCount; i++)
    {
        int len = write(fd,  sValue.data(), sValue.size());
        if (len != sValue.size())
        {
            printf("write fail, writelen %d\n", len);
            close(fd);
            return;
        }
        fsync(fd);
    }

    uint64_t llEndTimeMs = GetTimestampMS();
    int iRunTimeMs = llEndTimeMs - llBeginTimeMs;
    int qps = (uint64_t)iWriteCount * 1000 / iRunTimeMs;

    printf("qps %d\n", qps);
    close(fd);
}

int main(int argc, char ** argv)
{
    vector<string> filelist;
    for (int i = 2; i < 30; i++)
    {
        if (argc >= i)
        {
            filelist.push_back(argv[u - 1]);
        }
        else
        {
            break;
        }
    }

    for (size_t i = 0; i < filelist.size(); i++)
    {
        if (fork() == 0)
        {
            benchfsync(filelist[i]);
            exit(0);
        }
    }

    return 0;
}
