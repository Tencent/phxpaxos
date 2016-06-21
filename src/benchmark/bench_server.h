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

#pragma once

#include "phxpaxos/node.h"
#include "bench_sm.h"
#include <string>
#include <vector>

namespace bench
{

class BenchServer
{
public:
    BenchServer(const int iGroupCount, const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList);
    ~BenchServer();

    int RunPaxos();

    int ReadyBench();

    int Write(const std::string & sBenchValue);

    int Write(const int iGroupIdx, const std::string & sBenchValue);

private:
    int MakeLogStoragePath(std::string & sLogStoragePath);

private:
    phxpaxos::NodeInfo m_oMyNode;
    phxpaxos::NodeInfoList m_vecNodeList;

    int m_iGroupCount;
    std::vector<BenchSM *> m_vecSMList;

    phxpaxos::Node * m_poPaxosNode;
};
    
}


