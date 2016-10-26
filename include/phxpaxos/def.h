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

namespace phxpaxos
{

#define SYSTEM_V_SMID 100000000
#define MASTER_V_SMID 100000001
#define BATCH_PROPOSE_SMID 100000002

enum PaxosTryCommitRet
{
    PaxosTryCommitRet_OK = 0,
    PaxosTryCommitRet_Reject = -2,
    PaxosTryCommitRet_Conflict = 14,
    PaxosTryCommitRet_ExecuteFail = 15,
    PaxosTryCommitRet_Follower_Cannot_Commit = 16,
    PaxosTryCommitRet_Im_Not_In_Membership  = 17,
    PaxosTryCommitRet_Value_Size_TooLarge = 18,
    PaxosTryCommitRet_Timeout = 404,
    PaxosTryCommitRet_TooManyThreadWaiting_Reject = 405,
};

enum PaxosNodeFunctionRet
{
    Paxos_SystemError = -1,
    Paxos_GroupIdxWrong = -5,
    Paxos_MembershipOp_GidNotSame = -501,
    Paxos_MembershipOp_VersionConflit = -502,
    Paxos_MembershipOp_NoGid = 1001,
    Paxos_MembershipOp_Add_NodeExist = 1002,
    Paxos_MembershipOp_Remove_NodeNotExist = 1003,
    Paxos_MembershipOp_Change_NoChange = 1004,
    Paxos_GetInstanceValue_Value_NotExist = 1005,
    Paxos_GetInstanceValue_Value_Not_Chosen_Yet = 1006,
};

}

