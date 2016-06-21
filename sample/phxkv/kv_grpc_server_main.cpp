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

#include "kv_grpc_server.h"
#include <iostream>
#include <memory>
#include <string>
#include "log.h"

#include <grpc++/grpc++.h>

using namespace phxpaxos;
using namespace phxkv;
using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

int parse_ipport(const char * pcStr, NodeInfo & oNodeInfo)
{
    char sIP[32] = {0};
    int iPort = -1;

    int count = sscanf(pcStr, "%[^':']:%d", sIP, &iPort);
    if (count != 2)
    {
        return -1;
    }

    oNodeInfo.SetIPPort(sIP, iPort);

    return 0;
}

int parse_ipport_list(const char * pcStr, NodeInfoList & vecNodeInfoList)
{
    string sTmpStr;
    int iStrLen = strlen(pcStr);

    for (int i = 0; i < iStrLen; i++)
    {
        if (pcStr[i] == ',' || i == iStrLen - 1)
        {
            if (i == iStrLen - 1 && pcStr[i] != ',')
            {
                sTmpStr += pcStr[i];
            }
            
            NodeInfo oNodeInfo;
            int ret = parse_ipport(sTmpStr.c_str(), oNodeInfo);
            if (ret != 0)
            {
                return ret;
            }

            vecNodeInfoList.push_back(oNodeInfo);

            sTmpStr = "";
        }
        else
        {
            sTmpStr += pcStr[i];
        }
    }

    return 0;
}

int main(int argc, char ** argv)
{
    if (argc < 6)
    {
        printf("%s <grpc myip:myport> <paxos myip:myport> <node0_ip:node_0port,node1_ip:node_1_port,node2_ip:node2_port,...> <kvdb storagepath> <paxoslog storagepath>\n", argv[0]);
        return -1;
    }

    string sServerAddress = argv[1];

    NodeInfo oMyNode;
    if (parse_ipport(argv[2], oMyNode) != 0)
    {
        printf("parse myip:myport fail\n");
        return -1;
    }

    NodeInfoList vecNodeInfoList;
    if (parse_ipport_list(argv[3], vecNodeInfoList) != 0)
    {
        printf("parse ip/port list fail\n");
        return -1;
    }

    string sKVDBPath = argv[4];
    string sPaxosLogPath = argv[5];

    int ret = LOGGER->Init("phxkv", "./log", 3);
    if (ret != 0)
    {
        printf("server log init fail, ret %d\n", ret);
        return ret;
    }

    NLDebug("server init start.............................");

    PhxKVServiceImpl oPhxKVServer(oMyNode, vecNodeInfoList, sKVDBPath, sPaxosLogPath);
    ret = oPhxKVServer.Init();
    if (ret != 0)
    {
        printf("server init fail, ret %d\n", ret);
        return ret;
    }

    NLDebug("server init ok.............................");

    ServerBuilder oBuilder;

    oBuilder.AddListeningPort(sServerAddress, grpc::InsecureServerCredentials());

    oBuilder.RegisterService(&oPhxKVServer);

    std::unique_ptr<Server> server(oBuilder.BuildAndStart());
    std::cout << "Server listening on " << sServerAddress << std::endl;

    server->Wait();

    return 0;
}


