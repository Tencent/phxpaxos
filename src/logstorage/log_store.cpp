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
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "crc32.h"
#include "comm_include.h"
#include "db.h"
#include "paxos_msg.pb.h"

namespace phxpaxos
{

LogStore :: LogStore()
{
    m_iFd = -1;
    m_iMetaFd = -1;
    m_iFileID = -1;
    m_iDeletedMaxFileID = -1;
    m_iMyGroupIdx = -1;
    m_iNowFileSize = -1;
    m_iNowFileOffset = 0;
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

int LogStore :: Init(const std::string & sPath, const int iMyGroupIdx, Database * poDatabase)
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

    m_oFileLogger.Init(m_sPath);

    string sMetaFilePath = m_sPath + "/meta";
    
    m_iMetaFd = open(sMetaFilePath.c_str(), O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
    if (m_iMetaFd == -1)
    {
        PLG1Err("open meta file fail, filepath %s", sMetaFilePath.c_str());
        return -1;
    }

    off_t iSeekPos = lseek(m_iMetaFd, 0, SEEK_SET);
    if (iSeekPos == -1)
    {
        return -1;
    }

    ssize_t iReadLen = read(m_iMetaFd, &m_iFileID, sizeof(int));
    if (iReadLen != (ssize_t)sizeof(int))
    {
        if (iReadLen == 0)
        {
            m_iFileID = 0;
        }
        else
        {
            PLG1Err("read meta info fail, readlen %zd", iReadLen);
            return -1;
        }
    }

    uint32_t iMetaChecksum = 0;
    iReadLen = read(m_iMetaFd, &iMetaChecksum, sizeof(uint32_t));
    if (iReadLen == (ssize_t)sizeof(uint32_t))
    {
        uint32_t iCheckSum = crc32(0, (const uint8_t*)(&m_iFileID), sizeof(int));
        if (iCheckSum != iMetaChecksum)
        {
            PLG1Err("meta file checksum %u not same to cal checksum %u, fileid %d",
                    iMetaChecksum, iCheckSum, m_iFileID);
            return -2;
        }
    }

    int ret = RebuildIndex(poDatabase, m_iNowFileOffset);
    if (ret != 0)
    {
        PLG1Err("rebuild index fail, ret %d", ret);
        return -1;
    }

    ret = OpenFile(m_iFileID, m_iFd);
    if (ret != 0)
    {
        return ret;
    }

    ret = ExpandFile(m_iFd, m_iNowFileSize);
    if (ret != 0)
    {
        return ret;
    }

    m_iNowFileOffset = lseek(m_iFd, m_iNowFileOffset, SEEK_SET);
    if (m_iNowFileOffset == -1)
    {
        PLG1Err("seek to now file offset %d fail", m_iNowFileOffset);
        return -1;
    }

    m_oFileLogger.Log("init write fileid %d now_w_offset %d filesize %d", 
            m_iFileID, m_iNowFileOffset, m_iNowFileSize);

    PLG1Head("ok, path %s fileid %d meta checksum %u nowfilesize %d nowfilewriteoffset %d", 
            m_sPath.c_str(), m_iFileID, iMetaChecksum, m_iNowFileSize, m_iNowFileOffset);

    return 0;
}

int LogStore :: ExpandFile(int iFd, int & iFileSize)
{
    iFileSize = lseek(iFd, 0, SEEK_END);
    if (iFileSize == -1)
    {
        PLG1Err("lseek fail, ret %d", iFileSize);
        return -1;
    }

    if (iFileSize == 0)
    {
        //new file
        iFileSize = lseek(iFd, LOG_FILE_MAX_SIZE - 1, SEEK_SET);
        if (iFileSize != LOG_FILE_MAX_SIZE - 1)
        {
            return -1;
        }

        ssize_t iWriteLen = write(iFd, "\0", 1);
        if (iWriteLen != 1)
        {
            PLG1Err("write 1 bytes fail");
            return -1;
        }

        iFileSize = LOG_FILE_MAX_SIZE;
        int iOffset = lseek(iFd, 0, SEEK_SET);
        m_iNowFileOffset = 0;
        if (iOffset != 0)
        {
            return -1;
        }
    }

    return 0;
}

int LogStore :: IncreaseFileID()
{
    int iFileID = m_iFileID + 1;
    uint32_t iCheckSum = crc32(0, (const uint8_t*)(&iFileID), sizeof(int));

    off_t iSeekPos = lseek(m_iMetaFd, 0, SEEK_SET);
    if (iSeekPos == -1)
    {
        return -1;
    }

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

    int ret = fsync(m_iMetaFd);
    if (ret != 0)
    {
        return -1;
    }

    m_iFileID++;

    return 0;
}

int LogStore :: OpenFile(const int iFileID, int & iFd)
{
    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", m_sPath.c_str(), iFileID);
    iFd = open(sFilePath, O_CREAT | O_RDWR, S_IWRITE | S_IREAD);
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
    if (m_iDeletedMaxFileID == -1)
    {
        if (iFileID - 2000 > 0)
        {
            m_iDeletedMaxFileID = iFileID - 2000;
        }
    }

    if (iFileID <= m_iDeletedMaxFileID)
    {
        PLG1Debug("file already deleted, fileid %d deletedmaxfileid %d", iFileID, m_iDeletedMaxFileID);
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
            PLG1Debug("file already deleted, filepath %s", sFilePath);
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
        m_oFileLogger.Log("delete fileid %d", iDeleteFileID);
    }

    return ret;
}

int LogStore :: GetFileFD(const int iNeedWriteSize, int & iFd, int & iFileID, int & iOffset)
{
    if (m_iFd == -1)
    {
        PLG1Err("File aready broken, fileid %d", m_iFileID);
        return -1;
    }

    iOffset = lseek(m_iFd, m_iNowFileOffset, SEEK_SET);
    assert(iOffset != -1);

    if (iOffset + iNeedWriteSize > m_iNowFileSize)
    {
        close(m_iFd);
        m_iFd = -1;

        int ret = IncreaseFileID();
        if (ret != 0)
        {
            m_oFileLogger.Log("new file increase fileid fail, now fileid %d", m_iFileID);
            return ret;
        }

        ret = OpenFile(m_iFileID, m_iFd);
        if (ret != 0)
        {
            m_oFileLogger.Log("new file open file fail, now fileid %d", m_iFileID);
            return ret;
        }

        iOffset = lseek(m_iFd, 0, SEEK_END);
        if (iOffset != 0)
        {
            assert(iOffset != -1);

            m_oFileLogger.Log("new file but file aready exist, now fileid %d exist filesize %d", 
                    m_iFileID, iOffset);

            PLG1Err("IncreaseFileID success, but file exist, data wrong, file size %d", iOffset);
            assert(false);
            return -1;
        }

        ret = ExpandFile(m_iFd, m_iNowFileSize);
        if (ret != 0)
        {
            PLG1Err("new file expand fail, fileid %d fd %d", m_iFileID, m_iFd);

            m_oFileLogger.Log("new file expand file fail, now fileid %d", m_iFileID);

            close(m_iFd);
            m_iFd = -1;
            return -1;
        }

        m_oFileLogger.Log("new file expand ok, fileid %d filesize %d", m_iFileID, m_iNowFileSize);
    }

    iFd = m_iFd;
    iFileID = m_iFileID;

    return 0;
}

int LogStore :: Append(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sBuffer, std::string & sFileID)
{
    m_oTimeStat.Point();
    std::lock_guard<std::mutex> oLock(m_oMutex);

    int iFd = -1;
    int iFileID = -1;
    int iOffset = -1;

    int iLen = sizeof(uint64_t) + sBuffer.size();
    int iTmpBufferLen = iLen + sizeof(int);

    int ret = GetFileFD(iTmpBufferLen, iFd, iFileID, iOffset);
    if (ret != 0)
    {
        return ret;
    }

    m_oTmpAppendBuffer.Ready(iTmpBufferLen);

    memcpy(m_oTmpAppendBuffer.GetPtr(), &iLen, sizeof(int));
    memcpy(m_oTmpAppendBuffer.GetPtr() + sizeof(int), &llInstanceID, sizeof(uint64_t));
    memcpy(m_oTmpAppendBuffer.GetPtr() + sizeof(int) + sizeof(uint64_t), sBuffer.c_str(), sBuffer.size());

    size_t iWriteLen = write(iFd, m_oTmpAppendBuffer.GetPtr(), iTmpBufferLen);

    if (iWriteLen != (size_t)iTmpBufferLen)
    {
        BP->GetLogStorageBP()->AppendDataFail();
        PLG1Err("writelen %d not equal to %d, buffersize %zu errno %d", 
                iWriteLen, iTmpBufferLen, sBuffer.size(), errno);
        return -1;
    }

    if (oWriteOptions.bSync)
    {
        int fdatasync_ret = fdatasync(iFd);
        if (fdatasync_ret == -1)
        {
            PLG1Err("fdatasync fail, writelen %zu errno %d", iWriteLen, errno);
            return -1;
        }
    }

    m_iNowFileOffset += iWriteLen;

    int iUseTimeMs = m_oTimeStat.Point();
    BP->GetLogStorageBP()->AppendDataOK(iWriteLen, iUseTimeMs);
    
    uint32_t iCheckSum = crc32(0, (const uint8_t*)(m_oTmpAppendBuffer.GetPtr() + sizeof(int)), iTmpBufferLen - sizeof(int), CRC32SKIP);

    GenFileID(iFileID, iOffset, iCheckSum, sFileID);

    PLG1Imp("ok, offset %d fileid %d checksum %u instanceid %lu buffer size %zu usetime %dms sync %d",
            iOffset, iFileID, iCheckSum, llInstanceID, sBuffer.size(), iUseTimeMs, (int)oWriteOptions.bSync);

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
    
    off_t iSeekPos = lseek(iFd, iOffset, SEEK_SET);
    if (iSeekPos == -1)
    {
        return -1;
    }
    
    int iLen = 0;
    ssize_t iReadLen = read(iFd, (char *)&iLen, sizeof(int));
    if (iReadLen != (ssize_t)sizeof(int))
    {
        close(iFd);
        PLG1Err("readlen %zd not qual to %zu", iReadLen, sizeof(int));
        return -1;
    }
    
    std::lock_guard<std::mutex> oLock(m_oReadMutex);

    m_oTmpBuffer.Ready(iLen);
    iReadLen = read(iFd, m_oTmpBuffer.GetPtr(), iLen);
    if (iReadLen != iLen)
    {
        close(iFd);
        PLG1Err("readlen %zd not qual to %zu", iReadLen, iLen);
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

int LogStore :: RebuildIndex(Database * poDatabase, int & iNowFileWriteOffset)
{
    string sLastFileID;

    uint64_t llNowInstanceID = 0;
    int ret = poDatabase->GetMaxInstanceIDFileID(sLastFileID, llNowInstanceID);
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
        ret = RebuildIndexForOneFile(iNowFileID, iOffset, poDatabase, iNowFileWriteOffset, llNowInstanceID);
        if (ret != 0 && ret != 1)
        {
            break;
        }
        else if (ret == 1)
        {
            if (iNowFileID != 0 && iNowFileID != m_iFileID + 1)
            {
                PLG1Err("meta file wrong, nowfileid %d meta.nowfileid %d", iNowFileID, m_iFileID);
                return -1;
            }

            ret = 0;
            PLG1Imp("END rebuild ok, nowfileid %d", iNowFileID);
            break;
        }

        iOffset = 0;
    }
    
    return ret;
}

int LogStore :: RebuildIndexForOneFile(const int iFileID, const int iOffset, 
        Database * poDatabase, int & iNowFileWriteOffset, uint64_t & llNowInstanceID)
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

    int iFileLen = lseek(iFd, 0, SEEK_END);
    if (iFileLen == -1)
    {
        close(iFd);
        return -1;
    }
    
    off_t iSeekPos = lseek(iFd, iOffset, SEEK_SET);
    if (iSeekPos == -1)
    {
        close(iFd);
        return -1;
    }

    int iNowOffset = iOffset;
    bool bNeedTruncate = false;

    while (true)
    {
        int iLen = 0;
        ssize_t iReadLen = read(iFd, (char *)&iLen, sizeof(int));
        if (iReadLen == 0)
        {
            PLG1Head("File End, fileid %d offset %d", iFileID, iNowOffset);
            iNowFileWriteOffset = iNowOffset;
            break;
        }
        
        if (iReadLen != (ssize_t)sizeof(int))
        {
            bNeedTruncate = true;
            PLG1Err("readlen %zd not qual to %zu, need truncate", iReadLen, sizeof(int));
            break;
        }

        if (iLen == 0)
        {
            PLG1Head("File Data End, fileid %d offset %d", iFileID, iNowOffset);
            iNowFileWriteOffset = iNowOffset;
            break;
        }

        if (iLen > iFileLen || iLen < (int)sizeof(uint64_t))
        {
            PLG1Err("File data len wrong, data len %d filelen %d",
                    iLen, iFileLen);
            ret = -1;
            break;
        }

        m_oTmpBuffer.Ready(iLen);
        iReadLen = read(iFd, m_oTmpBuffer.GetPtr(), iLen);
        if (iReadLen != iLen)
        {
            bNeedTruncate = true;
            PLG1Err("readlen %zd not qual to %zu, need truncate", iReadLen, iLen);
            break;
        }


        uint64_t llInstanceID = 0;
        memcpy(&llInstanceID, m_oTmpBuffer.GetPtr(), sizeof(uint64_t));

        //InstanceID must be ascending order.
        if (llInstanceID < llNowInstanceID)
        {
            PLG1Err("File data wrong, read instanceid %lu smaller than now instanceid %lu",
                    llInstanceID, llNowInstanceID);
            ret = -1;
            break;
        }
        llNowInstanceID = llInstanceID;

        AcceptorStateData oState;
        bool bBufferValid = oState.ParseFromArray(m_oTmpBuffer.GetPtr() + sizeof(uint64_t), iLen - sizeof(uint64_t));
        if (!bBufferValid)
        {
            m_iNowFileOffset = iNowOffset;
            PLG1Err("This instance's buffer wrong, can't parse to acceptState, instanceid %lu bufferlen %d nowoffset %d",
                    llInstanceID, iLen - sizeof(uint64_t), iNowOffset);
            bNeedTruncate = true;
            break;
        }

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
        m_oFileLogger.Log("truncate fileid %d offset %d filesize %d", 
                iFileID, iNowOffset, iFileLen);
        if (truncate(sFilePath, iNowOffset) != 0)
        {
            PLG1Err("truncate fail, file path %s truncate to length %d errno %d", 
                    sFilePath, iNowOffset, errno);
            return -1;
        }
    }

    return ret;
}

//////////////////////////////////////////////////////////

LogStoreLogger :: LogStoreLogger()
    : m_iLogFd(-1)
{
}

LogStoreLogger :: ~LogStoreLogger()
{
    if (m_iLogFd != -1)
    {
        close(m_iLogFd);
    }
}

void LogStoreLogger :: Init(const std::string & sPath)
{
    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/LOG", sPath.c_str());
    m_iLogFd = open(sFilePath, O_CREAT | O_RDWR | O_APPEND, S_IWRITE | S_IREAD);
}

void LogStoreLogger :: Log(const char * pcFormat, ...)
{
    if (m_iLogFd == -1)
    {
        return;
    }

    uint64_t llNowTime = Time::GetTimestampMS();
    time_t tNowTimeSeconds = (time_t)(llNowTime / 1000);
    tm * local_time = localtime(&tNowTimeSeconds);
    char sTimePrefix[64] = {0};
    strftime(sTimePrefix, sizeof(sTimePrefix), "%Y-%m-%d %H:%M:%S", local_time);

    char sPrefix[128] = {0};
    snprintf(sPrefix, sizeof(sPrefix), "%s:%d ", sTimePrefix, (int)(llNowTime % 1000));
    string sNewFormat = string(sPrefix) + pcFormat + "\n";

    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), sNewFormat.c_str(), args);
    va_end(args);

    int iLen = strnlen(sBuf, sizeof(sBuf));
    ssize_t iWriteLen = write(m_iLogFd, sBuf, iLen);
    if (iWriteLen != iLen)
    {
        PLErr("fail, len %d writelen %d", iLen, iWriteLen);
    }
}
    
}


