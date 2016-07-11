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

#include "base.h"
#include "msg_transport.h"
#include "instance.h"
#include "crc32.h"

namespace phxpaxos 
{

Base :: Base(const Config * poConfig, const MsgTransport * poMsgTransport, const Instance * poInstance)
{
    m_poConfig = (Config *)poConfig;
    m_poMsgTransport = (MsgTransport *)poMsgTransport;
    m_poInstance = (Instance *)poInstance;

    m_llInstanceID = 0;

    m_bIsTestMode = false;
}

Base :: ~Base()
{
}

uint64_t Base :: GetInstanceID()
{
    return m_llInstanceID;
}

void Base :: SetInstanceID(const uint64_t llInstanceID)
{
    m_llInstanceID = llInstanceID;
}

void Base :: NewInstance()
{
    m_llInstanceID++;
    InitForNewPaxosInstance();
}

const uint32_t Base :: GetLastChecksum() const
{
    return m_poInstance->GetLastChecksum();
}

int Base :: PackMsg(const PaxosMsg & oPaxosMsg, std::string & sBuffer)
{
    std::string sBodyBuffer;
    bool bSucc = oPaxosMsg.SerializeToString(&sBodyBuffer);
    if (!bSucc)
    {
        PLGErr("PaxosMsg.SerializeToString fail, skip this msg");
        return -1;
    }

    int iCmd = MsgCmd_PaxosMsg;
    PackBaseMsg(sBodyBuffer, iCmd, sBuffer);

    return 0;
}

int Base :: PackCheckpointMsg(const CheckpointMsg & oCheckpointMsg, std::string & sBuffer)
{
    std::string sBodyBuffer;
    bool bSucc = oCheckpointMsg.SerializeToString(&sBodyBuffer);
    if (!bSucc)
    {
        PLGErr("CheckpointMsg.SerializeToString fail, skip this msg");
        return -1;
    }

    int iCmd = MsgCmd_CheckpointMsg;
    PackBaseMsg(sBodyBuffer, iCmd, sBuffer);

    return 0;
}

void Base :: PackBaseMsg(const std::string & sBodyBuffer, const int iCmd, std::string & sBuffer)
{
    char sGroupIdx[GROUPIDXLEN] = {0};
    int iGroupIdx = m_poConfig->GetMyGroupIdx();
    memcpy(sGroupIdx, &iGroupIdx, sizeof(sGroupIdx));

    Header oHeader;
    oHeader.set_gid(m_poConfig->GetGid());
    oHeader.set_rid(0);
    oHeader.set_cmdid(iCmd);
    oHeader.set_version(1);

    std::string sHeaderBuffer;
    bool bSucc = oHeader.SerializeToString(&sHeaderBuffer);
    if (!bSucc)
    {
        PLGErr("Header.SerializeToString fail, skip this msg");
        assert(bSucc == true);
    }

    char sHeaderLen[HEADLEN_LEN] = {0};
    uint16_t iHeaderLen = (uint16_t)sHeaderBuffer.size();
    memcpy(sHeaderLen, &iHeaderLen, sizeof(sHeaderLen));

    sBuffer = string(sGroupIdx, sizeof(sGroupIdx)) + string(sHeaderLen, sizeof(sHeaderLen)) + sHeaderBuffer + sBodyBuffer;

    //check sum
    uint32_t iBufferChecksum = crc32(0, (const uint8_t *)sBuffer.data(), sBuffer.size(), NET_CRC32SKIP);
    char sBufferChecksum[CHECKSUM_LEN] = {0};
    memcpy(sBufferChecksum, &iBufferChecksum, sizeof(sBufferChecksum));

    sBuffer += string(sBufferChecksum, sizeof(sBufferChecksum));
}

int Base :: UnPackBaseMsg(const std::string & sBuffer, Header & oHeader, size_t & iBodyStartPos, size_t & iBodyLen)
{
    uint16_t iHeaderLen = 0;
    memcpy(&iHeaderLen, sBuffer.data() + GROUPIDXLEN, HEADLEN_LEN);

    size_t iHeaderStartPos = GROUPIDXLEN + HEADLEN_LEN;
    iBodyStartPos = iHeaderStartPos + iHeaderLen;

    if (iBodyStartPos > sBuffer.size())
    {
        BP->GetAlgorithmBaseBP()->UnPackHeaderLenTooLong();
        NLErr("Header headerlen too loog %d", iHeaderLen);
        return -1;
    }

    bool bSucc = oHeader.ParseFromArray(sBuffer.data() + iHeaderStartPos, iHeaderLen);
    if (!bSucc)
    {
        NLErr("Header.ParseFromArray fail, skip this msg");
        return -1;
    }

    NLDebug("buffer_size %zu header len %d cmdid %d gid %lu rid %lu version %d body_startpos %zu", 
            sBuffer.size(), iHeaderLen, oHeader.cmdid(), oHeader.gid(), oHeader.rid(), oHeader.version(), iBodyStartPos);

    if (oHeader.version() >= 1)
    {
        if (iBodyStartPos + CHECKSUM_LEN > sBuffer.size())
        {
            NLErr("no checksum, body start pos %zu buffersize %zu", iBodyStartPos, sBuffer.size());
            return -1;
        }

        iBodyLen = sBuffer.size() - CHECKSUM_LEN - iBodyStartPos;

        uint32_t iBufferChecksum = 0;
        memcpy(&iBufferChecksum, sBuffer.data() + sBuffer.size() - CHECKSUM_LEN, CHECKSUM_LEN);
        
        uint32_t iNewCalBufferChecksum = crc32(0, (const uint8_t *)sBuffer.data(), sBuffer.size() - CHECKSUM_LEN, NET_CRC32SKIP);
        if (iNewCalBufferChecksum != iBufferChecksum)
        {
            BP->GetAlgorithmBaseBP()->UnPackChecksumNotSame();
            NLErr("Data.bring.checksum %u not equal to Data.cal.checksum %u",
                    iBufferChecksum, iNewCalBufferChecksum);
            return -1;
        }

        /*
        NLDebug("Checksum compare ok, Data.bring.checksum %u, Data.cal.checksum %u",
                iBufferChecksum, iNewCalBufferChecksum) 
        */
    }
    else
    {
        iBodyLen = sBuffer.size() - iBodyStartPos;
    }

    return 0;
}

int Base :: SendMessage(const nodeid_t iSendtoNodeID, const CheckpointMsg & oCheckpointMsg, const int iSendType)
{
    if (iSendtoNodeID == m_poConfig->GetMyNodeID())
    {
        return 0; 
    }
    
    string sBuffer;
    int ret = PackCheckpointMsg(oCheckpointMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return m_poMsgTransport->SendMessage(iSendtoNodeID, sBuffer, iSendType);
}

int Base :: SendMessage(const nodeid_t iSendtoNodeID, const PaxosMsg & oPaxosMsg, const int iSendType)
{
    if (m_bIsTestMode)
    {
        return 0;
    }

    BP->GetInstanceBP()->SendMessage();

    if (iSendtoNodeID == m_poConfig->GetMyNodeID())
    {
        m_poInstance->OnReceivePaxosMsg(oPaxosMsg);
        return 0; 
    }
    
    string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return m_poMsgTransport->SendMessage(iSendtoNodeID, sBuffer, iSendType);
}

int Base :: BroadcastMessage(const PaxosMsg & oPaxosMsg, const int iRunType, const int iSendType)
{
    if (m_bIsTestMode)
    {
        return 0;
    }

    BP->GetInstanceBP()->BroadcastMessage();

    if (iRunType == BroadcastMessage_Type_RunSelf_First)
    {
        if (m_poInstance->OnReceivePaxosMsg(oPaxosMsg) != 0)
        {
            return -1;
        }
    }
    
    string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    ret = m_poMsgTransport->BroadcastMessage(sBuffer, iSendType);

    if (iRunType == BroadcastMessage_Type_RunSelf_Final)
    {
        m_poInstance->OnReceivePaxosMsg(oPaxosMsg);
    }

    return ret;
}

int Base :: BroadcastMessageToFollower(const PaxosMsg & oPaxosMsg, const int iSendType)
{
    string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return m_poMsgTransport->BroadcastMessageFollower(sBuffer, iSendType);
}

int Base :: BroadcastMessageToTempNode(const PaxosMsg & oPaxosMsg, const int iSendType)
{
    string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return m_poMsgTransport->BroadcastMessageTempNode(sBuffer, iSendType);
}

///////////////////////////

void Base :: SetAsTestMode()
{
    m_bIsTestMode = true;
}
    
}


