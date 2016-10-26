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
#include "commdef.h"
#include "utils_include.h"

namespace phxpaxos
{

int PaxosComparator :: Compare(const leveldb::Slice & a, const leveldb::Slice & b) const
{
    return PCompare(a, b);
}

int PaxosComparator :: PCompare(const leveldb::Slice & a, const leveldb::Slice & b) 
{
    if (a.size() != sizeof(uint64_t))
    {
        NLErr("assert a.size %zu b.size %zu", a.size(), b.size());
        assert(a.size() == sizeof(uint64_t));
    }

    if (b.size() != sizeof(uint64_t))
    {
        NLErr("assert a.size %zu b.size %zu", a.size(), b.size());
        assert(b.size() == sizeof(uint64_t));
    }
    
    uint64_t lla = 0;
    uint64_t llb = 0;

    memcpy(&lla, a.data(), sizeof(uint64_t));
    memcpy(&llb, b.data(), sizeof(uint64_t));

    if (lla == llb)
    {
        return 0;
    }

    return lla < llb ? -1 : 1;
}

////////////////////////

Database :: Database() : m_poLevelDB(nullptr), m_poValueStore(nullptr)
{
    m_bHasInit = false;
    m_iMyGroupIdx = -1;
}

Database :: ~Database()
{
    delete m_poValueStore;
    delete m_poLevelDB;

    PLG1Head("LevelDB Deleted. Path %s", m_sDBPath.c_str());
}

int Database :: ClearAllLog()
{
    string sSystemVariablesBuffer;
    int ret = GetSystemVariables(sSystemVariablesBuffer);
    if (ret != 0 && ret != 1)
    {
        PLG1Err("GetSystemVariables fail, ret %d", ret);
        return ret;
    }

    string sMasterVariablesBuffer;
    ret = GetMasterVariables(sMasterVariablesBuffer);
    if (ret != 0 && ret != 1)
    {
        PLG1Err("GetMasterVariables fail, ret %d", ret);
        return ret;
    }

    m_bHasInit = false;

    delete m_poLevelDB;
    m_poLevelDB = nullptr;

    delete m_poValueStore;
    m_poValueStore = nullptr;

    string sBakPath = m_sDBPath + ".bak";

    ret = FileUtils::DeleteDir(sBakPath);
    if (ret != 0)
    {
        PLG1Err("Delete bak dir fail, dir %s", sBakPath.c_str());
        return -1;
    }

    ret = rename(m_sDBPath.c_str(), sBakPath.c_str());
    assert(ret == 0);

    ret = Init(m_sDBPath, m_iMyGroupIdx);
    if (ret != 0)
    {
        PLG1Err("Init again fail, ret %d", ret);
        return ret;
    }

    WriteOptions oWriteOptions;
    oWriteOptions.bSync = true;
    if (sSystemVariablesBuffer.size() > 0)
    {
        ret = SetSystemVariables(oWriteOptions, sSystemVariablesBuffer);
        if (ret != 0)
        {
            PLG1Err("SetSystemVariables fail, ret %d", ret);
            return ret;
        }
    }

    if (sMasterVariablesBuffer.size() > 0)
    {
        ret = SetMasterVariables(oWriteOptions, sMasterVariablesBuffer);
        if (ret != 0)
        {
            PLG1Err("SetMasterVariables fail, ret %d", ret);
            return ret;
        }
    }

    return 0;
}

int Database :: Init(const std::string & sDBPath, const int iMyGroupIdx)
{
    if (m_bHasInit)
    {
        return 0;
    }

    m_iMyGroupIdx = iMyGroupIdx;

    m_sDBPath = sDBPath;
    
    leveldb::Options oOptions;
    oOptions.create_if_missing = true;
    oOptions.comparator = &m_oPaxosCmp;
    //every group have different buffer size to avoid all group compact at the same time.
    oOptions.write_buffer_size = 1024 * 1024 + iMyGroupIdx * 10 * 1024;

    leveldb::Status oStatus = leveldb::DB::Open(oOptions, sDBPath, &m_poLevelDB);

    if (!oStatus.ok())
    {
        PLG1Err("Open leveldb fail, db_path %s", sDBPath.c_str());
        return -1;
    }

    m_poValueStore = new LogStore(); 
    assert(m_poValueStore != nullptr);

    int ret = m_poValueStore->Init(sDBPath, iMyGroupIdx, (Database *)this);
    if (ret != 0)
    {
        PLG1Err("value store init fail, ret %d", ret);
        return -1;
    }

    m_bHasInit = true;

    PLG1Imp("OK, db_path %s", sDBPath.c_str());

    return 0;
}

const std::string Database :: GetDBPath()
{
    return m_sDBPath;
}

int Database :: GetMaxInstanceIDFileID(std::string & sFileID, uint64_t & llInstanceID)
{
    uint64_t llMaxInstanceID = 0;
    int ret = GetMaxInstanceID(llMaxInstanceID);
    if (ret != 0 && ret != 1)
    {
        return ret;
    }

    if (ret == 1)
    {
        sFileID = "";
        return 0;
    }

    string sKey = GenKey(llMaxInstanceID);
    
    leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sFileID);
    if (!oStatus.ok())
    {
        if (oStatus.IsNotFound())
        {
            BP->GetLogStorageBP()->LevelDBGetNotExist();
            //PLG1Err("LevelDB.Get not found %s", sKey.c_str());
            return 1;
        }
        
        BP->GetLogStorageBP()->LevelDBGetFail();
        PLG1Err("LevelDB.Get fail");
        return -1;
    }

    llInstanceID = llMaxInstanceID;

    return 0;
}

int Database :: RebuildOneIndex(const uint64_t llInstanceID, const std::string & sFileID)
{
    string sKey = GenKey(llInstanceID);

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = false;

    leveldb::Status oStatus = m_poLevelDB->Put(oLevelDBWriteOptions, sKey, sFileID);
    if (!oStatus.ok())
    {
        BP->GetLogStorageBP()->LevelDBPutFail();
        PLG1Err("LevelDB.Put fail, instanceid %lu valuelen %zu", llInstanceID, sFileID.size());
        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////

int Database :: GetFromLevelDB(const uint64_t llInstanceID, std::string & sValue)
{
    string sKey = GenKey(llInstanceID);
    
    leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sValue);
    if (!oStatus.ok())
    {
        if (oStatus.IsNotFound())
        {
            BP->GetLogStorageBP()->LevelDBGetNotExist();
            PLG1Debug("LevelDB.Get not found, instanceid %lu", llInstanceID);
            return 1;
        }
        
        BP->GetLogStorageBP()->LevelDBGetFail();
        PLG1Err("LevelDB.Get fail, instanceid %lu", llInstanceID);
        return -1;
    }

    return 0;
}

int Database :: Get(const uint64_t llInstanceID, std::string & sValue)
{
    if (!m_bHasInit)
    {
        PLG1Err("no init yet");
        return -1;
    }

    string sFileID;
    int ret = GetFromLevelDB(llInstanceID, sFileID);
    if (ret != 0)
    {
        return ret;
    }

    uint64_t llFileInstanceID = 0;
    ret = FileIDToValue(sFileID, llFileInstanceID, sValue);
    if (ret != 0)
    {
        BP->GetLogStorageBP()->FileIDToValueFail();
        return ret;
    }

    if (llFileInstanceID != llInstanceID)
    {
        PLG1Err("file instanceid %lu not equal to key.instanceid %lu", llFileInstanceID, llInstanceID);
        return -2;
    }

    return 0;
}

int Database :: ValueToFileID(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sValue, std::string & sFileID)
{
    int ret = m_poValueStore->Append(oWriteOptions, llInstanceID, sValue, sFileID);
    if (ret != 0)
    {
        BP->GetLogStorageBP()->ValueToFileIDFail();
        PLG1Err("fail, ret %d", ret);
        return ret;
    }

    return 0;
}

int Database :: FileIDToValue(const std::string & sFileID, uint64_t & llInstanceID, std::string & sValue)
{
    int ret = m_poValueStore->Read(sFileID, llInstanceID, sValue);
    if (ret != 0)
    {
        PLG1Err("fail, ret %d", ret);
        return ret;
    }

    return 0;
}

int Database :: PutToLevelDB(const bool bSync, const uint64_t llInstanceID, const std::string & sValue)
{
    string sKey = GenKey(llInstanceID);

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = bSync;

    m_oTimeStat.Point();

    leveldb::Status oStatus = m_poLevelDB->Put(oLevelDBWriteOptions, sKey, sValue);
    if (!oStatus.ok())
    {
        BP->GetLogStorageBP()->LevelDBPutFail();
        PLG1Err("LevelDB.Put fail, instanceid %lu valuelen %zu", llInstanceID, sValue.size());
        return -1;
    }

    BP->GetLogStorageBP()->LevelDBPutOK(m_oTimeStat.Point());

    return 0;
}

int Database :: Put(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sValue)
{
    if (!m_bHasInit)
    {
        PLG1Err("no init yet");
        return -1;
    }

    std::string sFileID;
    int ret = ValueToFileID(oWriteOptions, llInstanceID, sValue, sFileID);
    if (ret != 0)
    {
        return ret;
    }

    ret = PutToLevelDB(false, llInstanceID, sFileID);
    
    return ret;
}

int Database :: ForceDel(const WriteOptions & oWriteOptions, const uint64_t llInstanceID)
{
    if (!m_bHasInit)
    {
        PLG1Err("no init yet");
        return -1;
    }

    string sKey = GenKey(llInstanceID);
    string sFileID;
    
    leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sFileID);
    if (!oStatus.ok())
    {
        if (oStatus.IsNotFound())
        {
            PLG1Debug("LevelDB.Get not found, instanceid %lu", llInstanceID);
            return 0;
        }
        
        PLG1Err("LevelDB.Get fail, instanceid %lu", llInstanceID);
        return -1;
    }

    int ret = m_poValueStore->ForceDel(sFileID, llInstanceID);
    if (ret != 0)
    {
        return ret;
    }

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = oWriteOptions.bSync;
    
    oStatus = m_poLevelDB->Delete(oLevelDBWriteOptions, sKey);
    if (!oStatus.ok())
    {
        PLG1Err("LevelDB.Delete fail, instanceid %lu", llInstanceID);
        return -1;
    }

    return 0;
}

int Database :: Del(const WriteOptions & oWriteOptions, const uint64_t llInstanceID)
{
    if (!m_bHasInit)
    {
        PLG1Err("no init yet");
        return -1;
    }

    string sKey = GenKey(llInstanceID);

    if (OtherUtils::FastRand() % 100 < 1)
    {
        //no need to del vfile every times.
        string sFileID;
        leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sFileID);
        if (!oStatus.ok())
        {
            if (oStatus.IsNotFound())
            {
                PLG1Debug("LevelDB.Get not found, instanceid %lu", llInstanceID);
                return 0;
            }
            
            PLG1Err("LevelDB.Get fail, instanceid %lu", llInstanceID);
            return -1;
        }

        int ret = m_poValueStore->Del(sFileID, llInstanceID);
        if (ret != 0)
        {
            return ret;
        }
    }

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = oWriteOptions.bSync;
    
    leveldb::Status oStatus = m_poLevelDB->Delete(oLevelDBWriteOptions, sKey);
    if (!oStatus.ok())
    {
        PLG1Err("LevelDB.Delete fail, instanceid %lu", llInstanceID);
        return -1;
    }

    return 0;
}

int Database :: GetMaxInstanceID(uint64_t & llInstanceID)
{
    llInstanceID = MINCHOSEN_KEY;

    leveldb::Iterator * it = m_poLevelDB->NewIterator(leveldb::ReadOptions());
    
    it->SeekToLast();

    while (it->Valid())
    {
        llInstanceID = GetInstanceIDFromKey(it->key().ToString());
        if (llInstanceID == MINCHOSEN_KEY
                || llInstanceID == SYSTEMVARIABLES_KEY
                || llInstanceID == MASTERVARIABLES_KEY)
        {
            it->Prev();
        }
        else
        {
            delete it;
            return 0;
        }
    }

    delete it;
    return 1;
}

std::string Database :: GenKey(const uint64_t llInstanceID)
{
    string sKey;
    sKey.append((char *)&llInstanceID, sizeof(uint64_t));
    return sKey;
}

const uint64_t Database :: GetInstanceIDFromKey(const std::string & sKey)
{
    uint64_t llInstanceID = 0;
    memcpy(&llInstanceID, sKey.data(), sizeof(uint64_t));

    return llInstanceID;
}

int Database :: GetMinChosenInstanceID(uint64_t & llMinInstanceID)
{
    if (!m_bHasInit)
    {
        PLG1Err("no init yet");
        return -1;
    }

    static uint64_t llMinKey = MINCHOSEN_KEY;
    std::string sValue;
    int ret = GetFromLevelDB(llMinKey, sValue);
    if (ret != 0 && ret != 1)
    {
        PLG1Err("fail, ret %d", ret);
        return ret;
    }

    if (ret == 1)
    {
        PLG1Err("no min chosen instanceid");
        llMinInstanceID = 0;
        return 0;
    }

    //old version, minchonsenid store in logstore.
    //new version, minchonsenid directly store in leveldb.
    if (m_poValueStore->IsValidFileID(sValue))
    {
        ret = Get(llMinKey, sValue);
        if (ret != 0 && ret != 1)
        {
            PLG1Err("Get from log store fail, ret %d", ret);
            return ret;
        }
    }

    if (sValue.size() != sizeof(uint64_t))
    {
        PLG1Err("fail, mininstanceid size wrong");
        return -2;
    }

    memcpy(&llMinInstanceID, sValue.data(), sizeof(uint64_t));

    PLG1Imp("ok, min chosen instanceid %lu", llMinInstanceID);

    return 0;
}

int Database :: SetMinChosenInstanceID(const WriteOptions & oWriteOptions, const uint64_t llMinInstanceID)
{
    if (!m_bHasInit)
    {
        PLG1Err("no init yet");
        return -1;
    }

    static uint64_t llMinKey = MINCHOSEN_KEY;
    char sValue[sizeof(uint64_t)] = {0};
    memcpy(sValue, &llMinInstanceID, sizeof(uint64_t));

    int ret = PutToLevelDB(true, llMinKey, string(sValue, sizeof(uint64_t)));
    if (ret != 0)
    {
        return ret;
    }

    PLG1Imp("ok, min chosen instanceid %lu", llMinInstanceID);

    return 0;
}


int Database :: SetSystemVariables(const WriteOptions & oWriteOptions, const std::string & sBuffer)
{
    static uint64_t llSystemVariablesKey = SYSTEMVARIABLES_KEY;
    return PutToLevelDB(true, llSystemVariablesKey, sBuffer);
}

int Database :: GetSystemVariables(std::string & sBuffer)
{
    static uint64_t llSystemVariablesKey = SYSTEMVARIABLES_KEY;
    return GetFromLevelDB(llSystemVariablesKey, sBuffer);
}

int Database :: SetMasterVariables(const WriteOptions & oWriteOptions, const std::string & sBuffer)
{
    static uint64_t llMasterVariablesKey = MASTERVARIABLES_KEY;
    return PutToLevelDB(true, llMasterVariablesKey, sBuffer);
}

int Database :: GetMasterVariables(std::string & sBuffer)
{
    static uint64_t llMasterVariablesKey = MASTERVARIABLES_KEY;
    return GetFromLevelDB(llMasterVariablesKey, sBuffer);
}

////////////////////////////////////////////////////

MultiDatabase :: MultiDatabase()
{
}

MultiDatabase :: ~MultiDatabase()
{
    for (auto & poDB : m_vecDBList)
    {
        delete poDB;
    }
}

int MultiDatabase :: Init(const std::string & sDBPath, const int iGroupCount)
{
    if (access(sDBPath.c_str(), F_OK) == -1)
    {
        PLErr("DBPath not exist or no limit to open, %s", sDBPath.c_str());
        return -1;
    }

    if (iGroupCount < 1 || iGroupCount > 100000)
    {
        PLErr("Groupcount wrong %d", iGroupCount);
        return -2;
    }

    std::string sNewDBPath = sDBPath;

    if (sDBPath[sDBPath.size() - 1] != '/')
    {
        sNewDBPath += '/';
    }

    for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++)
    {
        char sGroupDBPath[512] = {0};
        snprintf(sGroupDBPath, sizeof(sGroupDBPath), "%sg%d", sNewDBPath.c_str(), iGroupIdx);

        Database * poDB = new Database();
        assert(poDB != nullptr);
        m_vecDBList.push_back(poDB);

        if (poDB->Init(sGroupDBPath, iGroupIdx) != 0)
        {
            return -1;
        }
    }

    PLImp("OK, DBPath %s groupcount %d", sDBPath.c_str(), iGroupCount);

    return 0;
}

const std::string MultiDatabase :: GetLogStorageDirPath(const int iGroupIdx)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return "";
    }

    return m_vecDBList[iGroupIdx]->GetDBPath();
}

int MultiDatabase :: Get(const int iGroupIdx, const uint64_t llInstanceID, std::string & sValue)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->Get(llInstanceID, sValue);
}

int MultiDatabase :: Put(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID, const std::string & sValue)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }
    
    return m_vecDBList[iGroupIdx]->Put(oWriteOptions, llInstanceID, sValue);
}

int MultiDatabase :: Del(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }
    
    return m_vecDBList[iGroupIdx]->Del(oWriteOptions, llInstanceID);
}

int MultiDatabase :: ForceDel(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }
    
    return m_vecDBList[iGroupIdx]->ForceDel(oWriteOptions, llInstanceID);
}

int MultiDatabase :: GetMaxInstanceID(const int iGroupIdx, uint64_t & llInstanceID)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }
    
    return m_vecDBList[iGroupIdx]->GetMaxInstanceID(llInstanceID);
}
    
int MultiDatabase :: SetMinChosenInstanceID(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llMinInstanceID)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->SetMinChosenInstanceID(oWriteOptions, llMinInstanceID);
}

int MultiDatabase :: GetMinChosenInstanceID(const int iGroupIdx, uint64_t & llMinInstanceID)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->GetMinChosenInstanceID(llMinInstanceID);
}

int MultiDatabase :: ClearAllLog(const int iGroupIdx)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->ClearAllLog();
}

int MultiDatabase :: SetSystemVariables(const WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->SetSystemVariables(oWriteOptions, sBuffer);
}

int MultiDatabase :: GetSystemVariables(const int iGroupIdx, std::string & sBuffer)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->GetSystemVariables(sBuffer);
}

int MultiDatabase :: SetMasterVariables(const WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->SetMasterVariables(oWriteOptions, sBuffer);
}

int MultiDatabase :: GetMasterVariables(const int iGroupIdx, std::string & sBuffer)
{
    if (iGroupIdx >= (int)m_vecDBList.size())
    {
        return -2;
    }

    return m_vecDBList[iGroupIdx]->GetMasterVariables(sBuffer);
}

}


