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

#include "kv_grpc_client.h"
#include "phxpaxos/options.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace phxpaxos;
using namespace std;

namespace phxkv
{

PhxKVClient :: PhxKVClient(std::shared_ptr<Channel> channel)
    : stub_(PhxKVServer::NewStub(channel))
{
}

void PhxKVClient :: NewChannel(const uint64_t llNodeID)
{
    NodeInfo oNodeInfo(llNodeID);

    //for test multi node in one machine, each node have difference grpc_port.
    //but normally, every node's grpc_port is same, so you just set your grpc_port. 
    //if you change paxos_port/grpc_port's relation, you must modify this line.
    int iGrpcPort = oNodeInfo.GetPort() + 10000;

    char sAddress[128] = {0};
    snprintf(sAddress, sizeof(sAddress), "%s:%d", oNodeInfo.GetIP().c_str(), iGrpcPort);

    string sServerAddress = sAddress;

    printf("%s %s\n", __func__, sAddress);

    stub_.reset(nullptr);

    stub_ = PhxKVServer::NewStub(grpc::CreateChannel(sServerAddress,
                    grpc::InsecureChannelCredentials()));
}

int PhxKVClient :: Put(
        const std::string & sKey, 
        const std::string & sValue, 
        const uint64_t llVersion,
        const int iDeep)
{
    if (iDeep > 3)
    {
        return static_cast<int>(PhxKVStatus::FAIL);
    }

    KVOperator oRequest;
    oRequest.set_key(sKey);
    oRequest.set_value(sValue);
    oRequest.set_version(llVersion);
    oRequest.set_operator_(KVOperatorType_WRITE);

    KVResponse oResponse;
    ClientContext context;
    Status status = stub_->Put(&context, oRequest, &oResponse);

    if (status.ok())
    {
        if (oResponse.ret() == static_cast<int>(PhxKVStatus::MASTER_REDIRECT))
        {
            if (oResponse.master_nodeid() != phxpaxos::nullnode)
            {
                NewChannel(oResponse.master_nodeid());
                return Put(sKey, sValue, llVersion, iDeep + 1);
            }
            else
            {
                return static_cast<int>(PhxKVStatus::NO_MASTER);
            }
        }

        return oResponse.ret();
    }
    else
    {
        return static_cast<int>(PhxKVStatus::FAIL);
    }
}

int PhxKVClient :: GetLocal(
        const std::string & sKey, 
        std::string & sValue, 
        uint64_t & llVersion)
{
    KVOperator oRequest;
    oRequest.set_key(sKey);
    oRequest.set_operator_(KVOperatorType_READ);

    KVResponse oResponse;
    ClientContext context;
    Status status = stub_->GetLocal(&context, oRequest, &oResponse);

    if (status.ok())
    {
        sValue = oResponse.data().value();
        llVersion = oResponse.data().version();
        return oResponse.ret();
    }
    else
    {
        return static_cast<int>(PhxKVStatus::FAIL);
    }
}

int PhxKVClient :: GetLocal(
        const std::string & sKey, 
        const uint64_t minVersion, 
        std::string & sValue, 
        uint64_t & llVersion)
{
    int ret = GetLocal(sKey, sValue, llVersion);
    if (ret == 0)
    {
        if (llVersion < minVersion)
        {
            return static_cast<int>(PhxKVStatus::VERSION_NOTEXIST);
        }
    }

    return ret;
}

int PhxKVClient :: Delete( 
        const std::string & sKey, 
        const uint64_t llVersion,
        const int iDeep)
{
    KVOperator oRequest;
    oRequest.set_key(sKey);
    oRequest.set_version(llVersion);
    oRequest.set_operator_(KVOperatorType_DELETE);

    KVResponse oResponse;
    ClientContext context;
    Status status = stub_->Delete(&context, oRequest, &oResponse);

    if (status.ok())
    {
        if (oResponse.ret() == static_cast<int>(PhxKVStatus::MASTER_REDIRECT))
        {
            if (oResponse.master_nodeid() != phxpaxos::nullnode)
            {
                NewChannel(oResponse.master_nodeid());
                return Delete(sKey, llVersion, iDeep + 1);
            }
            else
            {
                return static_cast<int>(PhxKVStatus::NO_MASTER);
            }
        }

        return oResponse.ret();
    }
    else
    {
        return static_cast<int>(PhxKVStatus::FAIL);
    }
}

int PhxKVClient :: GetGlobal(
        const std::string & sKey, 
        std::string & sValue, 
        uint64_t & llVersion,
        const int iDeep)
{
    if (iDeep > 3)
    {
        return static_cast<int>(PhxKVStatus::FAIL);
    }

    KVOperator oRequest;
    oRequest.set_key(sKey);
    oRequest.set_operator_(KVOperatorType_READ);

    KVResponse oResponse;
    ClientContext context;
    Status status = stub_->GetGlobal(&context, oRequest, &oResponse);

    if (status.ok())
    {
        if (oResponse.ret() == static_cast<int>(PhxKVStatus::MASTER_REDIRECT))
        {
            if (oResponse.master_nodeid() != phxpaxos::nullnode)
            {
                NewChannel(oResponse.master_nodeid());
                return GetGlobal(sKey, sValue, llVersion, iDeep + 1);
            }
            else
            {
                return static_cast<int>(PhxKVStatus::NO_MASTER);
            }
        }

        sValue = oResponse.data().value();
        llVersion = oResponse.data().version();

        return oResponse.ret();
    }
    else
    {
        return static_cast<int>(PhxKVStatus::FAIL);
    }
}

}

