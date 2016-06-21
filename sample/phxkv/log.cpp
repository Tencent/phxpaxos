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

#include "log.h"

using namespace phxpaxos;

namespace phxkv
{

LoggerGuard :: LoggerGuard()
    : m_pLogFunc(nullptr)
{
}

LoggerGuard :: ~LoggerGuard()
{
}

LoggerGuard * LoggerGuard :: Instance()
{
    static LoggerGuard oLoggerGuard;
    return &oLoggerGuard;
}

int LoggerGuard :: Init(const std::string & sModuleName, const std::string & sLogPath, const int iLogLevel)
{
    int ret = LoggerGoogle :: GetLogger(sModuleName, sLogPath, iLogLevel, m_pLogFunc);
    if (ret != 0)
    {
        printf("get logger_google fail, ret %d\n", ret);
        return ret;
    }

    return 0;
}

phxpaxos::LogFunc LoggerGuard :: GetLogFunc()
{
    return m_pLogFunc;
}

    
}

