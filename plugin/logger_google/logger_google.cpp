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

#include "phxpaxos_plugin/logger_google.h"
#include <assert.h>
#include <stdarg.h>

#include <glog/logging.h>
#include <stdio.h>
#include <string>

using namespace std;

namespace phxpaxos
{

int LoggerGoogle :: GetLogger(const std::string & sModuleName, const std::string & sLogPath, const int iLogLevel, LogFunc & pLogFunc) 
{
    google::InitGoogleLogging(sModuleName.c_str());
    FLAGS_log_dir = sLogPath;
    FLAGS_stderrthreshold = google :: FATAL;
    switch( iLogLevel )
    {
        case 1: 
            FLAGS_minloglevel = google::ERROR;
            break;  
        case 2: 
            FLAGS_minloglevel = google::WARNING;
            break;  
        case 3: 
            FLAGS_minloglevel = google::INFO;
            break;  
    }

    LogError("%s", "init_glog_warning_file");

    pLogFunc = LoggerGoogle::Log;
    
    return 0;
}

void LoggerGoogle :: LogError(const char * pcFormat, ...)
{
    va_list args;
    va_start(args, pcFormat);
    Log(static_cast<int>(LogLevel::LogLevel_Error), pcFormat, args);
    va_end(args);
}

void LoggerGoogle :: LogWarning(const char * pcFormat, ...)
{
    va_list args;
    va_start(args, pcFormat);
    Log(static_cast<int>(LogLevel::LogLevel_Warning), pcFormat, args);
    va_end(args);
}

void LoggerGoogle :: LogInfo(const char * pcFormat, ...)
{
    va_list args;
    va_start(args, pcFormat);
    Log(static_cast<int>(LogLevel::LogLevel_Info), pcFormat, args);
    va_end(args);
}

void LoggerGoogle :: LogVerbose(const char * pcFormat, ...)
{
    va_list args;
    va_start(args, pcFormat);
    Log(static_cast<int>(LogLevel::LogLevel_Verbose), pcFormat, args);
    va_end(args);
}

void LoggerGoogle :: Log(const int iLogLevel, const char * pcFormat, va_list args)
{
    char sBuf[1024] = {0};
    vsnprintf(sBuf, sizeof(sBuf), pcFormat, args);

    if (iLogLevel == static_cast<int>(LogLevel::LogLevel_Error))
    {
        LOG(ERROR) << sBuf;
    }
    else if (iLogLevel == static_cast<int>(LogLevel::LogLevel_Warning))
    {
        LOG(WARNING) << sBuf;
    }
    else if (iLogLevel == static_cast<int>(LogLevel::LogLevel_Info))
    {
        LOG(INFO) << sBuf;
    }
    else if (iLogLevel == static_cast<int>(LogLevel::LogLevel_Verbose))
    {
        LOG(INFO) << sBuf;
    }
}
    
}


