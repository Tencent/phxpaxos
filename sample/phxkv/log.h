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

#include "phxpaxos_plugin/logger_google.h"
#include <string>
#include <typeinfo>

namespace phxkv
{

#define LOGGER (LoggerGuard::Instance())
#define LOG_ERROR(format, args...)\
       if (LOGGER->GetLogFunc() != nullptr)phxpaxos::LoggerGoogle::LogError(format, ## args);
#define LOG_WARNING(format, args...)\
       if (LOGGER->GetLogFunc() != nullptr)phxpaxos::LoggerGoogle::LogWarning(format, ## args);
#define LOG_INFO(format, args...)\
       if (LOGGER->GetLogFunc() != nullptr)phxpaxos::LoggerGoogle::LogInfo(format, ## args);
#define LOG_VERBOSE(format, args...)\
       if (LOGGER->GetLogFunc() != nullptr)phxpaxos::LoggerGoogle::LogVerbose(format, ## args);

#define NLDebug(format, args...) LOG_VERBOSE("DEBUG: %s " format, __func__, ## args);
#define NLErr(format, args...) LOG_ERROR("ERR: %s " format, __func__, ## args);

#define PLErr(format, args...) LOG_ERROR("ERR: %s::%s " format, typeid(this).name(), __func__, ## args);
#define PLImp(format, args...) LOG_INFO("Showy: %s::%s " format, typeid(this).name(), __func__, ## args);
#define PLHead(format, args...) LOG_WARNING("Imp: %s::%s " format, typeid(this).name(), __func__, ## args);
#define PLDebug(format, args...) LOG_VERBOSE("DEBUG: %s::%s " format, typeid(this).name(), __func__, ## args);

class LoggerGuard
{
public:
    LoggerGuard();
    ~LoggerGuard();

    int Init(const std::string & sModuleName, const std::string & sLogPath, const int iLogLevel);

    static LoggerGuard * Instance();

    phxpaxos::LogFunc GetLogFunc();

private:
    phxpaxos::LogFunc m_pLogFunc;
};
    
}
