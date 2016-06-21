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

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "phxkv.grpc.pb.h"

#include "def.h"

namespace phxkv
{

class PhxKVClient
{
public:
    PhxKVClient(std::shared_ptr<grpc::Channel> channel);

    void NewChannel(const uint64_t llNodeID);

    int Put(
            const std::string & sKey, 
            const std::string & sValue, 
            const uint64_t llVersion,
            const int iDeep = 0);

    int GetLocal(
            const std::string & sKey, 
            std::string & sValue, 
            uint64_t & llVersion);

    int GetLocal(
            const std::string & sKey, 
            const uint64_t minVersion, 
            std::string & sValue, 
            uint64_t & llVersion);

    int Delete( 
            const std::string & sKey, 
            const uint64_t llVersion,
            const int iDeep = 0);

    int GetGlobal(
            const std::string & sKey, 
            std::string & sValue, 
            uint64_t & llVersion,
            const int iDeep = 0);

private:
    std::unique_ptr<PhxKVServer::Stub> stub_;
};

}
