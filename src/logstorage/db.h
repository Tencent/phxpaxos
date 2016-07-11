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

#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include <vector>
#include <string>
#include <map>
#include "comm_include.h"
#include "phxpaxos/storage.h"
#include "log_store.h"

namespace phxpaxos
{

class PaxosComparator : public leveldb::Comparator
{
public:
    int Compare(const leveldb::Slice & a, const leveldb::Slice & b) const;
    
    static int PCompare(const leveldb::Slice & a, const leveldb::Slice & b);

    const char * Name() const {return "PaxosComparator";}

    void FindShortestSeparator(std::string *, const leveldb::Slice &) const {}

    void FindShortSuccessor(std::string *) const {}
};

//////////////////////////////////////////

#define MINCHOSEN_KEY ((uint64_t)-1)
#define SYSTEMVARIABLES_KEY ((uint64_t)-2)
#define MASTERVARIABLES_KEY ((uint64_t)-3)

class Database
{
public:
    Database();
    ~Database();

    int Init(const std::string & sDBPath, const int iMyGroupIdx);

    const std::string GetDBPath();

    int ClearAllLog();

    int Get(const uint64_t llInstanceID, std::string & sValue);

    int Put(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sValue);

    int Del(const WriteOptions & oWriteOptions, const uint64_t llInstanceID);

    int ForceDel(const WriteOptions & oWriteOptions, const uint64_t llInstanceID);

    int GetMaxInstanceID(uint64_t & llInstanceID);

    int SetMinChosenInstanceID(const WriteOptions & oWriteOptions, const uint64_t llMinInstanceID);

    int GetMinChosenInstanceID(uint64_t & llMinInstanceID);

    int SetSystemVariables(const WriteOptions & oWriteOptions, const std::string & sBuffer);

    int GetSystemVariables(std::string & sBuffer);

    int SetMasterVariables(const WriteOptions & oWriteOptions, const std::string & sBuffer);

    int GetMasterVariables(std::string & sBuffer);
    
public:
    int GetMaxInstanceIDFileID(std::string & sFileID, uint64_t & llInstanceID);

    int RebuildOneIndex(const uint64_t llInstanceID, const std::string & sFileID);
    
private:
    int ValueToFileID(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sValue, std::string & sFileID);

    int FileIDToValue(const std::string & sFileID, uint64_t & llInstanceID, std::string & sValue);

    int GetFromLevelDB(const uint64_t llInstanceID, std::string & sValue);

    int PutToLevelDB(const bool bSync, const uint64_t llInstanceID, const std::string & sValue);
        
private:
    std::string GenKey(const uint64_t llInstanceID);

    const uint64_t GetInstanceIDFromKey(const std::string & sKey);

public:
//private:
    leveldb::DB * m_poLevelDB;
    PaxosComparator m_oPaxosCmp;
    bool m_bHasInit;
    
    LogStore * m_poValueStore;
    std::string m_sDBPath;

    int m_iMyGroupIdx;

private:
    TimeStat m_oTimeStat;
};

//////////////////////////////////////////

class MultiDatabase : public LogStorage
{
public:
    MultiDatabase();
    ~MultiDatabase();

    int Init(const std::string & sDBPath, const int iGroupCount);

    const std::string GetLogStorageDirPath(const int iGroupIdx);

    int Get(const int iGroupIdx, const uint64_t llInstanceID, std::string & sValue);

    int Put(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID, const std::string & sValue);

    int Del(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID);

    int ForceDel(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID);

    int GetMaxInstanceID(const int iGroupIdx, uint64_t & llInstanceID);

    int SetMinChosenInstanceID(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llMinInstanceID);

    int GetMinChosenInstanceID(const int iGroupIdx, uint64_t & llMinInstanceID);

    int ClearAllLog(const int iGroupIdx);

    int SetSystemVariables(const WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer);

    int GetSystemVariables(const int iGroupIdx, std::string & sBuffer);
    
    int SetMasterVariables(const WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer);

    int GetMasterVariables(const int iGroupIdx, std::string & sBuffer);

private:
    std::vector<Database *> m_vecDBList;
};

}
    

