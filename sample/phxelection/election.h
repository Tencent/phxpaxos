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
#include "phxpaxos/options.h"
#include "phxpaxos_plugin/logger_google.h"
#include <string>
#include <vector>

namespace phxelection {

class PhxElection {
public:
  PhxElection(const phxpaxos::NodeInfo &oMyNode,
              const phxpaxos::NodeInfoList &vecNodeList);
  ~PhxElection();

  int RunPaxos();

  const phxpaxos::NodeInfo GetMaster();

  const phxpaxos::NodeInfo GetMasterWithVersion(uint64_t &llVersion);

  const bool IsIMMaster();

  static void OnMasterChange(const int iGroupIdx,
                             const phxpaxos::NodeInfo &oNewMaster,
                             const uint64_t llVersion);

private:
  int MakeLogStoragePath(std::string &sLogStoragePath);

private:
  phxpaxos::NodeInfo m_oMyNode;
  phxpaxos::NodeInfoList m_vecNodeList;

  phxpaxos::Node *m_poPaxosNode;
};

} // namespace phxelection
