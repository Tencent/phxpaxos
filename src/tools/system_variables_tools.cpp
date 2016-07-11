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
#include "config_include.h"

using namespace std;
using namespace phxpaxos;

void ShowVariables(SystemVSM & oVSM)
{
    SystemVariables oVariables;
    oVSM.GetSystemVariables(oVariables);
    
    printf("gid %llu\n", oVariables.gid());
    printf("version %llu\n", oVariables.version());

    for (int i = 0; i < oVariables.membership_size(); i++)
    {
        PaxosNodeInfo oNodeInfo = oVariables.membership(i);
        NodeInfo tTmpNode(oNodeInfo.nodeid());

        printf("ip %s port %d nodeid %lu\n", 
                tTmpNode.GetIP().c_str(), tTmpNode.GetPort(), tTmpNode.GetNodeID());
    }
}

void ModGid(SystemVSM & oVSM, const uint64_t llGid)
{
    SystemVariables oVariables;
    oVSM.GetSystemVariables(oVariables);

    oVariables.set_gid(llGid);

    int ret = oVSM.UpdateSystemVariables(oVariables);
    if (ret != 0)
    {
        printf("mod gid fail, ret %d gid %lu\n", ret, llGid);
    }
    else
    {
        printf("after mod gid:\n");
        ShowVariables(oVSM);
    }
}

int main(int argc, char ** argv)
{
    if (argc < 4)
    {
        printf("%s <paxos log dir> <group idx> <op> <gid/membership>\n", argv[0]);
        return -1;
    }

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

    SystemVSM oVSM(iGroupIdx, nullnode, &oDefaultLogStorage, nullptr);
    ret = oVSM.Init();
    if (ret != 0)
    {
        printf("system variable sm init fail, ret %d\n", ret);
        return ret;
    }

    string sOP = string(argv[3]);
    if (sOP == "show")
    {
        ShowVariables(oVSM);
    }
    else if (sOP == "modgid")
    {
        if (argc < 5)
        {
            printf("no gid to mod\n");
        }

        uint64_t llGid = strtoull(argv[4], NULL, 10);
        ModGid(oVSM, llGid);
    }

    return 0;
}


