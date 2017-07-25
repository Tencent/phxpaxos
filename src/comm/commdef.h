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

#include <inttypes.h>
#include <typeinfo>
#include "phxpaxos/options.h"
#include "inside_options.h"
#include "phxpaxos/def.h"
#include <string>
#include "logger.h"

using std::string;

namespace phxpaxos
{

#define NLDebug(format, args...) LOG_VERBOSE("DEBUG: %s " format, __func__, ## args);
#define NLErr(format, args...) LOG_ERROR("ERR: %s " format, __func__, ## args);

#define PLErr(format, args...) LOG_ERROR("ERR: %s::%s " format, typeid(this).name(), __func__, ## args);
#define PLImp(format, args...) LOG_SHOWY("Showy: %s::%s " format, typeid(this).name(), __func__, ## args);
#define PLHead(format, args...) LOG_WARNING("Imp: %s::%s " format, typeid(this).name(), __func__, ## args);
#define PLDebug(format, args...) LOG_VERBOSE("DEBUG: %s::%s " format, typeid(this).name(), __func__, ## args);

#define PLGErr(format, args...) LOG_ERROR("ERR(%d): %s::%s " format, m_poConfig->GetMyGroupIdx(), typeid(this).name(), __func__, ## args);
#define PLGStatus(format, args...) LOG_STATUS("STATUS(%d): %s::%s " format, m_poConfig->GetMyGroupIdx(), typeid(this).name(), __func__, ## args);
#define PLGImp(format, args...) LOG_SHOWY("Showy(%d): %s::%s " format, m_poConfig->GetMyGroupIdx(), typeid(this).name(), __func__, ## args);
#define PLGHead(format, args...) LOG_WARNING("Imp(%d): %s::%s " format, m_poConfig->GetMyGroupIdx(), typeid(this).name(), __func__, ## args);
#define PLGDebug(format, args...) LOG_VERBOSE("DEBUG(%d): %s::%s " format, m_poConfig->GetMyGroupIdx(), typeid(this).name(), __func__, ## args);
    
#define PLG1Err(format, args...) LOG_ERROR("ERR(%d): %s::%s " format, m_iMyGroupIdx, typeid(this).name(), __func__, ## args);
#define PLG1Imp(format, args...) LOG_SHOWY("Showy(%d): %s::%s " format, m_iMyGroupIdx, typeid(this).name(), __func__, ## args);
#define PLG1Head(format, args...) LOG_WARNING("Imp(%d): %s::%s " format, m_iMyGroupIdx, typeid(this).name(), __func__, ## args);
#define PLG1Debug(format, args...) LOG_VERBOSE("DEBUG(%d): %s::%s " format, m_iMyGroupIdx, typeid(this).name(), __func__, ## args);


#define nullvalue "nullvalue"

#define CRC32SKIP 8
#define NET_CRC32SKIP 7 

//network protocal
#define GROUPIDXLEN (sizeof(int))
#define HEADLEN_LEN (sizeof(uint16_t))
#define CHECKSUM_LEN (sizeof(uint32_t))

//max queue memsize
#define MAX_QUEUE_MEM_SIZE 209715200

enum MsgCmd
{
    MsgCmd_PaxosMsg = 1,
    MsgCmd_CheckpointMsg = 2,
};

enum PaxosMsgType
{
    MsgType_PaxosPrepare = 1,
    MsgType_PaxosPrepareReply = 2,
    MsgType_PaxosAccept = 3,
    MsgType_PaxosAcceptReply = 4,
    MsgType_PaxosLearner_AskforLearn = 5,
    MsgType_PaxosLearner_SendLearnValue = 6,
    MsgType_PaxosLearner_ProposerSendSuccess = 7,
    MsgType_PaxosProposal_SendNewValue = 8,
    MsgType_PaxosLearner_SendNowInstanceID = 9,
    MsgType_PaxosLearner_ComfirmAskforLearn = 10,
    MsgType_PaxosLearner_SendLearnValue_Ack = 11,
    MsgType_PaxosLearner_AskforCheckpoint = 12,
    MsgType_PaxosLearner_OnAskforCheckpoint = 13,
};

enum PaxosMsgFlagType
{
    PaxosMsgFlagType_SendLearnValue_NeedAck = 1,
};

enum CheckpointMsgType
{
    CheckpointMsgType_SendFile = 1,
    CheckpointMsgType_SendFile_Ack = 2,
};

enum CheckpointSendFileFlag
{
    CheckpointSendFileFlag_BEGIN = 1,
    CheckpointSendFileFlag_ING = 2,
    CheckpointSendFileFlag_END = 3,
};

enum CheckpointSendFileAckFlag
{
    CheckpointSendFileAckFlag_OK = 1,
    CheckpointSendFileAckFlag_Fail = 2,
};

enum TimerType
{
    Timer_Proposer_Prepare_Timeout = 1,
    Timer_Proposer_Accept_Timeout = 2,
    Timer_Learner_Askforlearn_noop = 3,
    Timer_Instance_Commit_Timeout = 4,
};

    
}
