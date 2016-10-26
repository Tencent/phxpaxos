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

#include "kv_grpc_server.h"

using grpc::ServerContext;
using grpc::Status;
using namespace std;

namespace phxkv
{


PhxKVServiceImpl :: PhxKVServiceImpl(const phxpaxos::NodeInfo & oMyNode, const phxpaxos::NodeInfoList & vecNodeList,
        const std::string & sKVDBPath, const std::string & sPaxosLogPath)
    : m_oPhxKV(oMyNode, vecNodeList, sKVDBPath, sPaxosLogPath)
{
}

int PhxKVServiceImpl :: Init()
{
    return m_oPhxKV.RunPaxos();
}

Status PhxKVServiceImpl :: Put(ServerContext* context, const KVOperator * request, KVResponse * reply)
{
    if (!m_oPhxKV.IsIMMaster(request->key()))
    {
        reply->set_ret((int)PhxKVStatus::MASTER_REDIRECT);
        uint64_t llMasterNodeID = m_oPhxKV.GetMaster(request->key()).GetNodeID();
        reply->set_master_nodeid(llMasterNodeID);

        PLImp("I'm not master, need redirect, master nodeid i saw %lu, key %s version %lu", 
                llMasterNodeID, request->key().c_str(), request->version());
        return Status::OK;
    }

    PhxKVStatus status = m_oPhxKV.Put(request->key(), request->value(), request->version());
    reply->set_ret((int)status);

    PLImp("ret %d, key %s version %lu", reply->ret(), request->key().c_str(), request->version());

    return Status::OK;
}

Status PhxKVServiceImpl :: GetLocal(ServerContext* context, const KVOperator * request, KVResponse * reply)
{
    string sReadValue;
    uint64_t llReadVersion = 0;

    PhxKVStatus status = m_oPhxKV.GetLocal(request->key(), sReadValue, llReadVersion);
    if (status == PhxKVStatus::SUCC)
    {
        reply->mutable_data()->set_value(sReadValue);
        reply->mutable_data()->set_version(llReadVersion);
    }
    else if (status == PhxKVStatus::KEY_NOTEXIST)
    {
        reply->mutable_data()->set_isdeleted(true);
        reply->mutable_data()->set_version(llReadVersion);
    }

    reply->set_ret((int)status);

    PLImp("ret %d, key %s version %lu", reply->ret(), request->key().c_str(), llReadVersion);

    return Status::OK;
}

Status PhxKVServiceImpl :: GetGlobal(ServerContext* context, const KVOperator * request, KVResponse * reply)
{
    if (!m_oPhxKV.IsIMMaster(request->key()))
    {
        reply->set_ret((int)PhxKVStatus::MASTER_REDIRECT);
        uint64_t llMasterNodeID = m_oPhxKV.GetMaster(request->key()).GetNodeID();
        reply->set_master_nodeid(llMasterNodeID);

        PLImp("I'm not master, need redirect, master nodeid i saw %lu, key %s version %lu", 
                llMasterNodeID, request->key().c_str(), request->version());

        return Status::OK;
    }

    return GetLocal(context, request, reply);
}

Status PhxKVServiceImpl :: Delete(ServerContext* context, const KVOperator * request, KVResponse * reply)
{
    if (!m_oPhxKV.IsIMMaster(request->key()))
    {
        reply->set_ret((int)PhxKVStatus::MASTER_REDIRECT);
        uint64_t llMasterNodeID = m_oPhxKV.GetMaster(request->key()).GetNodeID();
        reply->set_master_nodeid(llMasterNodeID);

        PLImp("I'm not master, need redirect, master nodeid i saw %lu, key %s version %lu", 
                llMasterNodeID, request->key().c_str(), request->version());

        return Status::OK;
    }

    PhxKVStatus status = m_oPhxKV.Delete(request->key(), request->version());
    reply->set_ret((int)status);

    PLImp("ret %d, key %s version %lu", reply->ret(), request->key().c_str(), request->version());

    return Status::OK;
}

}

