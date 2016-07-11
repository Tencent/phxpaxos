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

#include "checkpoint_receiver.h"
#include "comm_include.h"
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

namespace phxpaxos
{

CheckpointReceiver :: CheckpointReceiver(Config * poConfig, LogStorage * poLogStorage) :
    m_poConfig(poConfig), m_poLogStorage(poLogStorage)
{
    Reset();
}

CheckpointReceiver :: ~CheckpointReceiver()
{
}

void CheckpointReceiver :: Reset()
{
    m_mapHasInitDir.clear();
    
    m_iSenderNodeID = nullnode;
    m_llUUID = 0;
    m_llSequence = 0;
}

int CheckpointReceiver :: NewReceiver(const nodeid_t iSenderNodeID, const uint64_t llUUID)
{
    int ret = ClearCheckpointTmp();
    if (ret != 0)
    {
        return ret;
    }

    ret = m_poLogStorage->ClearAllLog(m_poConfig->GetMyGroupIdx());
    if (ret != 0)
    {
        PLGErr("ClearAllLog fail, groupidx %d ret %d", 
                m_poConfig->GetMyGroupIdx(), ret);
        return ret;
    }
    
    m_mapHasInitDir.clear();

    m_iSenderNodeID = iSenderNodeID;
    m_llUUID = llUUID;
    m_llSequence = 0;

    return 0;
}

int CheckpointReceiver :: ClearCheckpointTmp()
{
    string sLogStoragePath = m_poLogStorage->GetLogStorageDirPath(m_poConfig->GetMyGroupIdx());

    DIR * dir = nullptr;
    struct dirent  * ptr;

    dir = opendir(sLogStoragePath.c_str());
    if (dir == nullptr)
    {
        return -1;
    }

    int ret = 0;
    while ((ptr = readdir(dir)) != nullptr)
    {
        if (string(ptr->d_name).find("cp_tmp_") != std::string::npos)
        {
            char sChildPath[1024] = {0};
            snprintf(sChildPath, sizeof(sChildPath), "%s/%s", sLogStoragePath.c_str(), ptr->d_name);
            ret = FileUtils::DeleteDir(sChildPath);
            
            if (ret != 0)
            {
                break;
            }

            PLGHead("rm dir %s done!", sChildPath);
        }
    }

    closedir(dir);

    return ret;
}

const bool CheckpointReceiver :: IsReceiverFinish(const nodeid_t iSenderNodeID, 
        const uint64_t llUUID, const uint64_t llEndSequence)
{
    if (iSenderNodeID == m_iSenderNodeID
            && llUUID == m_llUUID
            && llEndSequence == m_llSequence + 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

const std::string CheckpointReceiver :: GetTmpDirPath(const int iSMID)
{
    string sLogStoragePath = m_poLogStorage->GetLogStorageDirPath(m_poConfig->GetMyGroupIdx());
    char sTmpDirPath[512] = {0};

    snprintf(sTmpDirPath, sizeof(sTmpDirPath), "%s/cp_tmp_%d", sLogStoragePath.c_str(), iSMID);

    return string(sTmpDirPath);
}

int CheckpointReceiver :: InitFilePath(const std::string & sFilePath, std::string & sFormatFilePath)
{
    PLGHead("START filepath %s", sFilePath.c_str());

    string sNewFilePath = "/" + sFilePath + "/";
    vector<std::string> vecDirList;

    std::string sDirName;
    for (size_t i = 0; i < sNewFilePath.size(); i++)
    {
        if (sNewFilePath[i] == '/')
        {
            if (sDirName.size() > 0)
            {
                vecDirList.push_back(sDirName);
            }

            sDirName = "";
        }
        else
        {
            sDirName += sNewFilePath[i];
        }
    }

    sFormatFilePath = "/";
    for (size_t i = 0; i < vecDirList.size(); i++)
    {
        if (i + 1 == vecDirList.size())
        {
            sFormatFilePath += vecDirList[i];
        }
        else
        {
            sFormatFilePath += vecDirList[i] + "/";
            if (m_mapHasInitDir.find(sFormatFilePath) == end(m_mapHasInitDir))
            {
                int ret = CreateDir(sFormatFilePath);
                if (ret != 0)
                {
                    return ret;
                }

                m_mapHasInitDir[sFormatFilePath] = true;
            }
        }
    }

    PLGImp("ok, format filepath %s", sFormatFilePath.c_str());

    return 0;
}

int CheckpointReceiver :: CreateDir(const std::string & sDirPath)
{
    if (access(sDirPath.c_str(), F_OK) == -1)
    {
        if (mkdir(sDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
        {       
            PLGErr("Create dir fail, path %s", sDirPath.c_str());
            return -1;
        }       
    }

    return 0;
}

int CheckpointReceiver :: ReceiveCheckpoint(const CheckpointMsg & oCheckpointMsg)
{
    if (oCheckpointMsg.nodeid() != m_iSenderNodeID
            || oCheckpointMsg.uuid() != m_llUUID)
    {
        PLGErr("msg not valid, Msg.SenderNodeID %lu Receiver.SenderNodeID %lu Msg.UUID %lu Receiver.UUID %lu",
                oCheckpointMsg.nodeid(), m_iSenderNodeID, oCheckpointMsg.uuid(), m_llUUID);
        return -2;
    }

    if (oCheckpointMsg.sequence() == m_llSequence)
    {
        PLGErr("msg already receive, skip, Msg.Sequence %lu Receiver.Sequence %lu",
                oCheckpointMsg.sequence(), m_llSequence);
        return 0;
    }

    if (oCheckpointMsg.sequence() != m_llSequence + 1)
    {
        PLGErr("msg sequence wrong, Msg.Sequence %lu Receiver.Sequence %lu",
                oCheckpointMsg.sequence(), m_llSequence);
        return -2;
    }

    string sFilePath = GetTmpDirPath(oCheckpointMsg.smid()) + "/" + oCheckpointMsg.filepath();
    string sFormatFilePath;
    int ret = InitFilePath(sFilePath, sFormatFilePath);
    if (ret != 0)
    {
        return -1;
    }

    int iFd = open(sFormatFilePath.c_str(), O_CREAT | O_RDWR | O_APPEND, S_IWRITE | S_IREAD);
    if (iFd == -1)
    {
        PLGErr("open file fail, filepath %s", sFormatFilePath.c_str());
        return -1;
    }

    size_t llFileOffset = lseek(iFd, 0, SEEK_END);
    if ((uint64_t)llFileOffset != oCheckpointMsg.offset())
    {
        PLGErr("file.offset %zu not equal to msg.offset %lu", llFileOffset, oCheckpointMsg.offset());
        close(iFd);
        return -2;
    }

    size_t iWriteLen = write(iFd, oCheckpointMsg.buffer().data(), oCheckpointMsg.buffer().size());
    if (iWriteLen != oCheckpointMsg.buffer().size())
    {
        PLGImp("write fail, writelen %zu buffer size %zu", iWriteLen, oCheckpointMsg.buffer().size());
        close(iFd);
        return -1;
    }

    m_llSequence++;
    close(iFd);

    PLGImp("END ok, writelen %zu", iWriteLen);

    return 0;
}
    
}


