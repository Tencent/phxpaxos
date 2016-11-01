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

#include <mutex>
#include "leveldb/db.h"
#include <string>

namespace phxkv
{

enum KVClientRet
{
    KVCLIENT_OK = 0,
    KVCLIENT_SYS_FAIL = -1,
    KVCLIENT_KEY_NOTEXIST = 1,
    KVCLIENT_KEY_VERSION_CONFLICT = -11,
};

#define KV_CHECKPOINT_KEY ((uint64_t)-1)

class KVClient
{
public:
    KVClient();
    ~KVClient();

    bool Init(const std::string & sDBPath);

    static KVClient * Instance();

    KVClientRet Get(const std::string & sKey, std::string & sValue, uint64_t & llVersion);

    KVClientRet Set(const std::string & sKey, const std::string & sValue, const uint64_t llVersion);

    KVClientRet Del(const std::string & sKey, const uint64_t llVersion);

    KVClientRet GetCheckpointInstanceID(uint64_t & llCheckpointInstanceID);

    KVClientRet SetCheckpointInstanceID(const uint64_t llCheckpointInstanceID);

private:
    leveldb::DB * m_poLevelDB;
    bool m_bHasInit;
    std::mutex m_oMutex;
};
    
}
