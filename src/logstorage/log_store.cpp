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

#include "log_store.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "crc32.h"
#include "comm_include.h"
#include "db.h"

namespace phxpaxos
{

LogStore :: LogStore()
{
    m_iFd = -1;
    m_iMetaFd = -1;
    m_iFileID = -1;
    m_iDeletedMaxFileID = -1;
}

LogStore :: ~LogStore()
{
    if (m_iFd != -1)
    {
        close(m_iFd);
    }

    if (m_iMetaFd != -1)
    {
        close(m_iMetaFd);
    }
}

int LogStore :: Init(const std::string & sPath, const int iMyGroupIdx)
{
    m_iMyGroupIdx = iMyGroupIdx;
    
    m_sPath = sPath + "/" + "vfile";

    if (access(m_sPath.c_str(), F_OK) == -1)
    {
        if (mkdir(m_sPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
        {
            PLG1Err("Create dir fail, path %s", m_sPath.c_str());
            return -1;
        }
    }

    string sMetaFilePath = m_sPath + "/meta";
    
    m_iMetaFd = open(sMetaFilePath.c_str(), O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
    if (m_iMetaFd == -1)
    {
        PLG1Err("open meta file fail, filepath %s", sMetaFilePath.c_str());
        return -1;
    }

    lseek(m_iMetaFd, 0, SEEK_SET);
    size_t iReadLen = read(m_iMetaFd, &m_iFileID, sizeof(int));
    if (iReadLen != sizeof(int))
    {
        if (iReadLen == 0)
        {
            m_iFileID = 0;
        }
        else
        {
            PLG1Err("read meta info fail, readlen %zu", iReadLen);
            return -1;
        }
    }

    uint32_t iMetaChecksum = 0;
    iReadLen = read(m_iMetaFd, &iMetaChecksum, sizeof(uint32_t));
    if (iReadLen == sizeof(uint32_t))
    {
        uint32_t iCheckSum = crc32(0, (const uint8_t*)(&m_iFileID), sizeof(int));
        if (iCheckSum != iMetaChecksum)
        {
            PLG1Err("meta file checksum %u not same to cal checksum %u, fileid %d",
                    iMetaChecksum, iCheckSum, m_iFileID);
            return -2;
        }
    }

    PLG1Head("ok, path %s fileid %d meta checksum %u", m_sPath.c_str(), m_iFileID, iMetaChecksum);

    int ret = OpenFile(m_iFileID, m_iFd);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int LogStore :: IncreaseFileID()
{
    int iFileID = m_iFileID + 1;
    uint32_t iCheckSum = crc32(0, (const uint8_t*)(&iFileID), sizeof(int));

    lseek(m_iMetaFd, 0, SEEK_SET);
    size_t iWriteLen = write(m_iMetaFd, (char *)&iFileID, sizeof(int));
    if (iWriteLen != sizeof(int))
    {
        PLG1Err("write meta fileid fail, writelen %zu", iWriteLen);
        return -1;
    }

    iWriteLen = write(m_iMetaFd, (char *)&iCheckSum, sizeof(uint32_t));
    if (iWriteLen != sizeof(uint32_t))
    {
        PLG1Err("write meta checksum fail, writelen %zu", iWriteLen);
        return -1;
    }

    fsync(m_iMetaFd);

    m_iFileID++;

    return 0;
}

int LogStore :: OpenFile(const int iFileID, int & iFd)
{
    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", m_sPath.c_str(), iFileID);
    iFd = open(sFilePath, O_CREAT | O_RDWR | O_APPEND, S_IWRITE | S_IREAD);
    if (iFd == -1)
    {
        PLG1Err("open fail fail, filepath %s", sFilePath);
        return -1;
    }

    PLG1Imp("ok, path %s", sFilePath);
    return 0;
}

int LogStore :: DeleteFile(const int iFileID)
{
    if (iFileID <= m_iDeletedMaxFileID)
    {
        PLG1Debug("file aready deleted, fileid %d deletedmaxfileid %d", iFileID, m_iDeletedMaxFileID);
        return 0;
    }
    
    int ret = 0;
    for (int iDeleteFileID = m_iDeletedMaxFileID + 1; iDeleteFileID <= iFileID; iDeleteFileID++)
    {
        char sFilePath[512] = {0};
        snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", m_sPath.c_str(), iDeleteFileID);

        ret = access(sFilePath, F_OK);
        if (ret == -1)
        {
            PLG1Err("file aready deleted, filepath %s", sFilePath);
            m_iDeletedMaxFileID = iDeleteFileID;
            ret = 0;
            continue;
        }

        ret = remove(sFilePath);
        if (ret != 0)
        {
            PLG1Err("remove fail, filepath %s ret %d", sFilePath, ret);
            break;
        }
        
        m_iDeletedMaxFileID = iDeleteFileID;
    }

    return ret;
}

int LogStore :: GetFileFD(int & iFd, int & iFileID, int & iOffset)
{
    iOffset = lseek(m_iFd, 0, SEEK_END);
    if (iOffset > LOG_FILE_MAX_SIZE)
    {
        close(m_iFd);
        m_iFd = -1;

        int ret = IncreaseFileID();
        if (ret != 0)
        {
            return ret;
        }

        ret = OpenFile(m_iFileID, m_iFd);
        if (ret != 0)
        {
            return ret;
        }

        iOffset = lseek(m_iFd, 0, SEEK_END);
        if (iOffset != 0)
        {
            PLG1Err("IncreaseFileID success, but file exist, data wrong, file size %d", iOffset);
            assert(false);
            return -1;
        }
    }

    iFd = m_iFd;
    iFileID = m_iFileID;

    return 0;
}

int LogStore :: Append(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sBuffer, std::string & sFileID)
{
    m_oTimeStat.Point();
    ScopedLock<Mutex> oLock(m_oMutex);

    int iFd = -1;
    int iFileID = -1;
    int iOffset = -1;

    int ret = GetFileFD(iFd, iFileID, iOffset);
    if (ret != 0)
    {
        return ret;
    }

    int iLen = sizeof(uint64_t) + sBuffer.size();
    int iTmpBufferLen = iLen + sizeof(int);

    m_oTmpAppendBuffer.Ready(iTmpBufferLen);

    memcpy(m_oTmpAppendBuffer.GetPtr(), &iLen, sizeof(int));
    memcpy(m_oTmpAppendBuffer.GetPtr() + sizeof(int), &llInstanceID, sizeof(uint64_t));
    memcpy(m_oTmpAppendBuffer.GetPtr() + sizeof(int) + sizeof(uint64_t), sBuffer.c_str(), sBuffer.size());

    size_t iWriteLen = write(iFd, m_oTmpAppendBuffer.GetPtr(), iTmpBufferLen);

    if (iWriteLen != (size_t)iTmpBufferLen)
    {
        BP->GetLogStorageBP()->AppendDataFail();
        PLG1Err("writelen %d not equal to %d, buffersize %zu", iWriteLen, iTmpBufferLen, sBuffer.size());
        return -1;
    }

    if (oWriteOptions.bSync)
    {
        fsync(iFd);
    }

    int iUseTimeMs = m_oTimeStat.Point();
    BP->GetLogStorageBP()->AppendDataOK(iWriteLen, iUseTimeMs);
    
    uint32_t iCheckSum = crc32(0, (const uint8_t*)(m_oTmpAppendBuffer.GetPtr() + sizeof(int)), iTmpBufferLen - sizeof(int), CRC32SKIP);

    GenFileID(iFileID, iOffset, iCheckSum, sFileID);

    PLG1Imp("ok, offset %d fileid %d checksum %u instanceid %lu buffer size %zu usetime %dms",
            iOffset, iFileID, iCheckSum, llInstanceID, sBuffer.size(), iUseTimeMs);

    return 0;
}

int LogStore :: Read(const std::string & sFileID, uint64_t & llInstanceID, std::string & sBuffer)
{
    int iFileID = -1;
    int iOffset = -1;
    uint32_t iCheckSum = 0;
    ParseFileID(sFileID, iFileID, iOffset, iCheckSum);
    
    int iFd = -1;
    int ret = OpenFile(iFileID, iFd);
    if (ret != 0)
    {
        return ret;
    }
    
    lseek(iFd, iOffset, SEEK_SET);
    
    int iLen = 0;
    size_t iReadLen = read(iFd, (char *)&iLen, sizeof(int));
    if (iReadLen != sizeof(int))
    {
        close(iFd);
        PLG1Err("readlen %d not qual to %zu", iReadLen, sizeof(int));
        return -1;
    }
    
    ScopedLock<Mutex> oLock(m_oReadMutex);

    m_oTmpBuffer.Ready(iLen);
    iReadLen = read(iFd, m_oTmpBuffer.GetPtr(), iLen);
    if (iReadLen != (size_t)iLen)
    {
        close(iFd);
        PLG1Err("readlen %d not qual to %zu", iReadLen, iLen);
        return -1;
    }

    close(iFd);

    uint32_t iFileCheckSum = crc32(0, (const uint8_t *)m_oTmpBuffer.GetPtr(), iLen, CRC32SKIP);

    if (iFileCheckSum != iCheckSum)
    {
        BP->GetLogStorageBP()->GetFileChecksumNotEquel();
        PLG1Err("checksum not equal, filechecksum %u checksum %u", iFileCheckSum, iCheckSum);
        return -2;
    }

    memcpy(&llInstanceID, m_oTmpBuffer.GetPtr(), sizeof(uint64_t));
    sBuffer = string(m_oTmpBuffer.GetPtr() + sizeof(uint64_t), iLen - sizeof(uint64_t));

    PLG1Imp("ok, fileid %d offset %d instanceid %lu buffer size %zu", 
            iFileID, iOffset, llInstanceID, sBuffer.size());

    return 0;
}

int LogStore :: Del(const std::string & sFileID, const uint64_t llInstanceID)
{
    int iFileID = -1;
    int iOffset = -1;
    uint32_t iCheckSum = 0;
    ParseFileID(sFileID, iFileID, iOffset, iCheckSum);

    if (iFileID > m_iFileID)
    {
        PLG1Err("del fileid %d large than useing fileid %d", iFileID, m_iFileID);
        return -2;
    }

    if (iFileID > 0)
    {
        return DeleteFile(iFileID - 1);
    }

    return 0;
}

int LogStore :: ForceDel(const std::string & sFileID, const uint64_t llInstanceID)
{
    int iFileID = -1;
    int iOffset = -1;
    uint32_t iCheckSum = 0;
    ParseFileID(sFileID, iFileID, iOffset, iCheckSum);

    if (iFileID != m_iFileID)
    {
        PLG1Err("del fileid %d not equal to fileid %d", iFileID, m_iFileID);
        return -2;
    }

    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", m_sPath.c_str(), iFileID);

    printf("fileid %d offset %d\n", iFileID, iOffset);

    if (truncate(sFilePath, iOffset) != 0)
    {
        return -1;
    }

    return 0;
}


void LogStore :: GenFileID(const int iFileID, const int iOffset, const uint32_t iCheckSum, std::string & sFileID)
{
    char sTmp[sizeof(int) + sizeof(int) + sizeof(uint32_t)] = {0};
    memcpy(sTmp, (char *)&iFileID, sizeof(int));
    memcpy(sTmp + sizeof(int), (char *)&iOffset, sizeof(int));
    memcpy(sTmp + sizeof(int) + sizeof(int), (char *)&iCheckSum, sizeof(uint32_t));

    sFileID = std::string(sTmp, sizeof(int) + sizeof(int) + sizeof(uint32_t));
}

void LogStore :: ParseFileID(const std::string & sFileID, int & iFileID, int & iOffset, uint32_t & iCheckSum)
{
    memcpy(&iFileID, (void *)sFileID.c_str(), sizeof(int));
    memcpy(&iOffset, (void *)(sFileID.c_str() + sizeof(int)), sizeof(int));
    memcpy(&iCheckSum, (void *)(sFileID.c_str() + sizeof(int) + sizeof(int)), sizeof(uint32_t));

    PLG1Debug("fileid %d offset %d checksum %u", iFileID, iOffset, iCheckSum);
}

const bool LogStore :: IsValidFileID(const std::string & sFileID)
{
    if (sFileID.size() != FILEID_LEN)
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////

int LogStore :: RebuildIndex(Database * poDatabase)
{
    string sLastFileID;

    int ret = poDatabase->GetMaxInstanceIDFileID(sLastFileID);
    if (ret != 0)
    {
        return ret;
    }

    int iFileID = 0;
    int iOffset = 0;
    uint32_t iCheckSum = 0;

    if (sLastFileID.size() > 0)
    {
        ParseFileID(sLastFileID, iFileID, iOffset, iCheckSum);
    }

    if (iFileID > m_iFileID)
    {
        PLG1Err("LevelDB last fileid %d larger than meta now fileid %d, file error",
                iFileID, m_iFileID);
        return -2;
    }

    PLG1Head("START fileid %d offset %d checksum %u", iFileID, iOffset, iCheckSum);

    for (int iNowFileID = iFileID; ;iNowFileID++)
    {
        ret = RebuildIndexForOneFile(iNowFileID, iOffset, poDatabase);
        if (ret != 0 && ret != 1)
        {
            break;
        }
        else if (ret == 1)
        {
            ret = 0;
            PLG1Imp("END rebuild ok, nowfileid %d", iNowFileID);
            break;
        }

        iOffset = 0;
    }
    
    return ret;
}

int LogStore :: RebuildIndexForOneFile(const int iFileID, const int iOffset, Database * poDatabase)
{
    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", m_sPath.c_str(), iFileID);

    int ret = access(sFilePath, F_OK);
    if (ret == -1)
    {
        PLG1Debug("file not exist, filepath %s", sFilePath);
        return 1;
    }

    int iFd = -1;
    ret = OpenFile(iFileID, iFd);
    if (ret != 0)
    {
        return ret;
    }
    
    lseek(iFd, iOffset, SEEK_SET);

    int iNowOffset = iOffset;
    bool bNeedTruncate = false;

    while (true)
    {
        int iLen = 0;
        size_t iReadLen = read(iFd, (char *)&iLen, sizeof(int));
        if (iReadLen == 0)
        {
            PLG1Head("File End, fileid %d offset %d", iFileID, iOffset);
            break;
        }
        
        if (iReadLen != sizeof(int))
        {
            bNeedTruncate = true;
            PLG1Err("readlen %d not qual to %zu, need truncate", iReadLen, sizeof(int));
            break;
        }

        m_oTmpBuffer.Ready(iLen);
        iReadLen = read(iFd, m_oTmpBuffer.GetPtr(), iLen);
        if (iReadLen != (size_t)iLen)
        {
            bNeedTruncate = true;
            PLG1Err("readlen %d not qual to %zu, need truncate", iReadLen, iLen);
            break;
        }


        uint64_t llInstanceID = 0;
        memcpy(&llInstanceID, m_oTmpBuffer.GetPtr(), sizeof(uint64_t));

        uint32_t iFileCheckSum = crc32(0, (const uint8_t *)m_oTmpBuffer.GetPtr(), iLen, CRC32SKIP);

        string sFileID;
        GenFileID(iFileID, iNowOffset, iFileCheckSum, sFileID);

        ret = poDatabase->RebuildOneIndex(llInstanceID, sFileID);
        if (ret != 0)
        {
            break;
        }

        PLG1Imp("rebuild one index ok, fileid %d offset %d instanceid %lu checksum %u buffer size %zu", 
                iFileID, iNowOffset, llInstanceID, iFileCheckSum, iLen - sizeof(uint64_t));

        iNowOffset += sizeof(int) + iLen; 
    }
    
    close(iFd);

    if (bNeedTruncate)
    {
        if (truncate(sFilePath, iNowOffset) != 0)
        {
            PLG1Err("truncate fail, file path %s truncate to length %d", sFilePath, iNowOffset);
            return -1;
        }
    }

    return ret;
}
    
}

