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

#include "kv.h"
#include "phxkv.pb.h"
#include "log.h"

using namespace phxpaxos;
using namespace std;

namespace phxkv
{

KVClient :: KVClient()
{
    m_bHasInit = false;
    m_poLevelDB = nullptr;
}

KVClient :: ~KVClient()
{
}

bool KVClient :: Init(const std::string & sDBPath)
{
    if (m_bHasInit)
    {
        return true;
    }
    
    leveldb::Options oOptions;
    oOptions.create_if_missing = true;
    leveldb::Status oStatus = leveldb::DB::Open(oOptions, sDBPath, &m_poLevelDB);

    if (!oStatus.ok())
    {
        PLErr("Open leveldb fail, db_path %s", sDBPath.c_str());
        return false;
    }

    m_bHasInit = true;

    PLImp("OK, db_path %s", sDBPath.c_str());

    return true;
}

KVClient * KVClient :: Instance()
{
    static KVClient oKVClient;
    return &oKVClient;
}

KVClientRet KVClient :: Get(const std::string & sKey, std::string & sValue, uint64_t & llVersion)
{
    if (!m_bHasInit)
    {
        PLErr("no init yet");
        return KVCLIENT_SYS_FAIL;
    }

    string sBuffer;
    leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sBuffer);
    if (!oStatus.ok())
    {
        if (oStatus.IsNotFound())
        {
            PLErr("LevelDB.Get not found, key %s", sKey.c_str());
            llVersion = 0;
            return KVCLIENT_KEY_NOTEXIST;
        }
        
        PLErr("LevelDB.Get fail, key %s", sKey.c_str());
        return KVCLIENT_SYS_FAIL;
    }

    KVData oData;
    bool bSucc = oData.ParseFromArray(sBuffer.data(), sBuffer.size());
    if (!bSucc)
    {
        PLErr("DB DATA wrong, key %s", sKey.c_str());
        return KVCLIENT_SYS_FAIL;
    }

    llVersion = oData.version();

    if (oData.isdeleted())
    {
        PLErr("LevelDB.Get key already deleted, key %s", sKey.c_str());
        return KVCLIENT_KEY_NOTEXIST;
    }

    sValue = oData.value();

    PLImp("OK, key %s value %s version %lu", sKey.c_str(), sValue.c_str(), llVersion);

    return KVCLIENT_OK;
}

KVClientRet KVClient :: Set(const std::string & sKey, const std::string & sValue, const uint64_t llVersion)
{
    if (!m_bHasInit)
    {
        PLErr("no init yet");
        return KVCLIENT_SYS_FAIL;
    }

    std::lock_guard<std::mutex> oLockGuard(m_oMutex);
    
    uint64_t llServerVersion = 0;
    std::string sServerValue;
    KVClientRet ret = Get(sKey, sServerValue, llServerVersion);
    if (ret != KVCLIENT_OK && ret != KVCLIENT_KEY_NOTEXIST)
    {
        return KVCLIENT_SYS_FAIL;
    }

    if (llServerVersion != llVersion)
    {
        return KVCLIENT_KEY_VERSION_CONFLICT;
    }

    llServerVersion++;
    KVData oData;
    oData.set_value(sValue);
    oData.set_version(llServerVersion);
    oData.set_isdeleted(false);

    std::string sBuffer;
    bool bSucc = oData.SerializeToString(&sBuffer);
    if (!bSucc)
    {
        PLErr("Data.SerializeToString fail");
        return KVCLIENT_SYS_FAIL;
    }
    
    leveldb::Status oStatus = m_poLevelDB->Put(leveldb::WriteOptions(), sKey, sBuffer);
    if (!oStatus.ok())
    {
        PLErr("LevelDB.Put fail, key %s bufferlen %zu", sKey.c_str(), sBuffer.size());
        return KVCLIENT_SYS_FAIL;
    }

    PLImp("OK, key %s value %s version %lu", sKey.c_str(), sValue.c_str(), llVersion);

    return KVCLIENT_OK;
}

KVClientRet KVClient :: Del(const std::string & sKey, const uint64_t llVersion)
{
    if (!m_bHasInit)
    {
        PLErr("no init yet");
        return KVCLIENT_SYS_FAIL;
    }

    std::lock_guard<std::mutex> oLockGuard(m_oMutex);
    
    uint64_t llServerVersion = 0;
    std::string sServerValue;
    KVClientRet ret = Get(sKey, sServerValue, llServerVersion);
    if (ret != KVCLIENT_OK && ret != KVCLIENT_KEY_NOTEXIST)
    {
        return KVCLIENT_SYS_FAIL;
    }

    if (llServerVersion != llVersion)
    {
        return KVCLIENT_KEY_VERSION_CONFLICT;
    }
    
    llServerVersion++;
    KVData oData;
    oData.set_value(sServerValue);
    oData.set_version(llServerVersion);
    oData.set_isdeleted(true);

    std::string sBuffer;
    bool bSucc = oData.SerializeToString(&sBuffer);
    if (!bSucc)
    {
        PLErr("Data.SerializeToString fail");
        return KVCLIENT_SYS_FAIL;
    }
    
    leveldb::Status oStatus = m_poLevelDB->Put(leveldb::WriteOptions(), sKey, sBuffer);
    if (!oStatus.ok())
    {
        PLErr("LevelDB.Put fail, key %s bufferlen %zu", sKey.c_str(), sBuffer.size());
        return KVCLIENT_SYS_FAIL;
    }

    PLImp("OK, key %s version %lu", sKey.c_str(), llVersion);

    return KVCLIENT_OK;
}

KVClientRet KVClient :: GetCheckpointInstanceID(uint64_t & llCheckpointInstanceID)
{
    if (!m_bHasInit)
    {
        PLErr("no init yet");
        return KVCLIENT_SYS_FAIL;
    }

    string sKey;
    static uint64_t llCheckpointInstanceIDKey = KV_CHECKPOINT_KEY;
    sKey.append((char *)&llCheckpointInstanceIDKey, sizeof(uint64_t));

    string sBuffer;
    leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sBuffer);
    if (!oStatus.ok())
    {
        if (oStatus.IsNotFound())
        {
            return KVCLIENT_KEY_NOTEXIST;
        }
        
        return KVCLIENT_SYS_FAIL;
    }

    memcpy(&llCheckpointInstanceID, sBuffer.data(), sizeof(uint64_t));

    PLImp("OK, CheckpointInstanceID %lu", llCheckpointInstanceID);

    return KVCLIENT_OK;
}

KVClientRet KVClient :: SetCheckpointInstanceID(const uint64_t llCheckpointInstanceID)
{
    if (!m_bHasInit)
    {
        PLErr("no init yet");
        return KVCLIENT_SYS_FAIL;
    }

    string sKey;
    static uint64_t llCheckpointInstanceIDKey = KV_CHECKPOINT_KEY;
    sKey.append((char *)&llCheckpointInstanceIDKey, sizeof(uint64_t));

    string sBuffer;
    sBuffer.append((char *)&llCheckpointInstanceID, sizeof(uint64_t));

    leveldb::WriteOptions oWriteOptions;
    //must fync
    oWriteOptions.sync = true;

    leveldb::Status oStatus = m_poLevelDB->Put(oWriteOptions, sKey, sBuffer);
    if (!oStatus.ok())
    {
        PLErr("LevelDB.Put fail, bufferlen %zu", sBuffer.size());
        return KVCLIENT_SYS_FAIL;
    }

    PLImp("OK, CheckpointInstanceID %lu", llCheckpointInstanceID);

    return KVCLIENT_OK;
}
    
}

