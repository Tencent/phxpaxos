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

#include <map>
#include <string>
#include "phxpaxos/options.h"
#include "comm_include.h"
#include "config_include.h"

namespace phxpaxos
{

class Config;
class LogStorage;

class CheckpointReceiver
{
public:
    CheckpointReceiver(Config * poConfig, LogStorage * poLogStorage);
    ~CheckpointReceiver();

    void Reset();

    int NewReceiver(const nodeid_t iSenderNodeID, const uint64_t llUUID);

    const bool IsReceiverFinish(const nodeid_t iSenderNodeID, const uint64_t llUUID, const uint64_t llEndSequence);

    const std::string GetTmpDirPath(const int iSMID);

    int ReceiveCheckpoint(const CheckpointMsg & oCheckpointMsg);

    int InitFilePath(const std::string & sFilePath, std::string & sFormatFilePath);
private:

    int ClearCheckpointTmp();

    int CreateDir(const std::string & sDirPath);

private:
    Config * m_poConfig;
    LogStorage * m_poLogStorage;

private:
    nodeid_t m_iSenderNodeID; 
    uint64_t m_llUUID;
    uint64_t m_llSequence;

private:
    std::map<std::string, bool> m_mapHasInitDir;
};
    
}
