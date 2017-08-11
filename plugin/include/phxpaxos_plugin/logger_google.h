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

#include <string>
#include <stdio.h>
#include "phxpaxos/log.h"

namespace phxpaxos
{

class LoggerGoogle
{
public:
    //LogLevel_None = 0,
    //LogLevel_Error = 1,
    //LogLevel_Warning = 2,
    //LogLevel_Info = 3,
    //LogLevel_Verbose = 4,
    static int GetLogger(const std::string & sModuleName, const std::string & sLogPath, const int iLogLevel, LogFunc & pLogFunc);

    static void Log(const int iLogLevel, const char * pcFormat, va_list args);

    static void LogError(const char * pcFormat, ...);

    static void LogWarning(const char * pcFormat, ...);

    static void LogInfo(const char * pcFormat, ...);

    static void LogVerbose(const char * pcFormat, ...);
};

}
