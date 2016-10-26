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

void benchfsync(const int iValueSize, const int iWriteCount)
{
    string sFilePath = "./bench_fsync_tmp_data.log";
    string sValue;
    RandValue(iValueSize, sValue);

    int fd = open(sFilePath.c_str(), O_CREAT | O_RDWR, S_IWRITE | S_IREAD);
    if (fd == -1)
    {
        printf("open file fail %s\n", sFilePath.c_str());
        return;
    }

    int ret = lseek(fd, 100 * 1024 * 1024, SEEK_CUR);
    if (ret == -1)
    {
        printf("lseek fail\n");
        return;
    }

    int wlen = write(fd, "\0", 1);
    if (wlen != 1)
    {
        printf("write 1 bytes fail, wlen %d\n", wlen);
        return;
    }

    lseek(fd, 0, SEEK_SET);

    uint64_t llBeginTimeMs = GetSteadyClockMS();
    for (int i = 0; i < iWriteCount; i++)
    {
        int len = write(fd,  sValue.data(), sValue.size());
        if (len != sValue.size())
        {
            printf("write fail, writelen %d\n", len);
            close(fd);
            return;
        }
        uint64_t llEnd = GetSteadyClockMS();
        fdatasync(fd);
    }

    uint64_t llEndTimeMs = GetSteadyClockMS();
    int iRunTimeMs = llEndTimeMs - llBeginTimeMs;
    int qps = (uint64_t)iWriteCount * 1000 / iRunTimeMs;

    printf("qps %d\n", qps);
    close(fd);
}

/*
int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("%s <value size> <write times>\n", argv[0]);
        return 0;
    }

    int iValueSize = atoi(argv[1]);
    int iWriteCount = atoi(argv[2]);
    benchfsync(iValueSize, iWriteCount);

    return 0;
}
*/

#define BUF_SIZE 512 
 
int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        printf("%s <value size> <write times>\n", argv[0]);
        return 0;
    }

    int iValueSize = atoi(argv[1]);
    int iWriteCount = atoi(argv[2]);

    int fd;
    int ret;
    unsigned char *buf;
    ret = posix_memalign((void **)&buf, 512, BUF_SIZE);
    if (ret) {
        perror("posix_memalign failed");
        exit(1);
    }
    memset(buf, 'c', BUF_SIZE);
 
    fd = open("./direct_io.data", O_WRONLY | O_DIRECT | O_CREAT /*| O_SYNC*/, 0755);
    if (fd < 0){
        perror("open ./direct_io.data failed");
        exit(1);
    }
 
    
    uint64_t llBeginTimeMs = GetSteadyClockMS();
    for (int i = 0; i < iWriteCount; i++)
    {
        uint64_t llBegin = GetSteadyClockMS();
        ret = write(fd, buf, BUF_SIZE);
        if (ret < 0) {
            perror("write ./direct_io.data failed");
        }
        uint64_t llEnd = GetSteadyClockMS();
        //printf("%dms\n", llEnd - llBegin);
    }
 
    free(buf);
    close(fd);

    uint64_t llEndTimeMs = GetSteadyClockMS();
    int iRunTimeMs = llEndTimeMs - llBeginTimeMs;
    int qps = (uint64_t)iWriteCount * 1000 / iRunTimeMs;

    printf("qps %d\n", qps);
}
