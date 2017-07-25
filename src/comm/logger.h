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

#include <mutex>
#include "phxpaxos/log.h"
#include "utils_include.h"
#include <string>

namespace phxpaxos
{

#define LOGGER (Logger::Instance())
#define LOG_ERROR(format, args...)\
       LOGGER->LogError(format, ## args);
#define LOG_STATUS(format, args...)\
       LOGGER->LogStatus(format, ## args);
#define LOG_WARNING(format, args...)\
       LOGGER->LogWarning(format, ## args);
#define LOG_INFO(format, args...)\
       LOGGER->LogInfo(format, ## args);
#define LOG_VERBOSE(format, args...)\
       LOGGER->LogVerbose(format, ## args);
#define LOG_SHOWY(format, args...)\
       LOGGER->LogShowy(format, ## args);

class Logger
{
public:
    Logger();
    ~Logger();

    static Logger * Instance();

    void InitLogger(const LogLevel eLogLevel);

    void SetLogFunc(LogFunc pLogFunc);

    void LogError(const char * pcFormat, ...);

    void LogStatus(const char * pcFormat, ...);

    void LogWarning(const char * pcFormat, ...);
    
    void LogInfo(const char * pcFormat, ...);
    
    void LogVerbose(const char * pcFormat, ...);

    void LogShowy(const char * pcFormat, ...);

private:
    LogFunc m_pLogFunc;
    LogLevel m_eLogLevel;
    std::mutex m_oMutex;
};
    
}
