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

#include "logger.h"
#include <string>
#include <stdarg.h>
using namespace std;

namespace phxpaxos
{

Logger :: Logger()
    : m_pLogFunc(nullptr), m_eLogLevel(LogLevel::LogLevel_None)
{
}

Logger :: ~Logger()
{
}

Logger * Logger :: Instance()
{
    static Logger oLogger;
    return &oLogger;
}

void Logger :: InitLogger(const LogLevel eLogLevel)
{
    m_eLogLevel = eLogLevel;
}

void Logger :: SetLogFunc(LogFunc pLogFunc)
{
    m_pLogFunc = pLogFunc;
}


void Logger :: LogError(const char * pcFormat, ...)
{
    string newFormat = "\033[41;37m " + string(pcFormat) + " \033[0m";

    if (m_pLogFunc != nullptr)
    {
        va_list args;
        va_start(args, pcFormat);
        m_pLogFunc(static_cast<int>(LogLevel::LogLevel_Error), newFormat.c_str(), args);
        va_end(args);
        return;
    }

    if (m_eLogLevel < LogLevel::LogLevel_Error)
    {
        return;
    }

    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
    va_end(args);

    m_oMutex.lock();
    printf("%s\n", sBuf);
    m_oMutex.unlock();
}

void Logger :: LogStatus(const char * pcFormat, ...)
{
    if (m_pLogFunc != nullptr)
    {
        va_list args;
        va_start(args, pcFormat);
        m_pLogFunc(static_cast<int>(LogLevel::LogLevel_Error), pcFormat, args);
        va_end(args);
        return;
    }

    if (m_eLogLevel < LogLevel::LogLevel_Error)
    {
        return;
    }

    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), pcFormat, args);
    va_end(args);

    m_oMutex.lock();
    printf("%s\n", sBuf);
    m_oMutex.unlock();
}

void Logger :: LogWarning(const char * pcFormat, ...)
{
    string newFormat = "\033[44;37m " + string(pcFormat) + " \033[0m";

    if (m_pLogFunc != nullptr)
    {
        va_list args;
        va_start(args, pcFormat);
        m_pLogFunc(static_cast<int>(LogLevel::LogLevel_Warning), newFormat.c_str(), args);
        va_end(args);
        return;
    }

    if (m_eLogLevel < LogLevel::LogLevel_Warning)
    {
        return;
    }
        
    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
    va_end(args);

    m_oMutex.lock();
    printf("%s\n", sBuf);
    m_oMutex.unlock();
}


void Logger :: LogInfo(const char * pcFormat, ...)
{
    string newFormat = "\033[45;37m " + string(pcFormat) + " \033[0m";

    if (m_pLogFunc != nullptr)
    {
        va_list args;
        va_start(args, pcFormat);
        m_pLogFunc(static_cast<int>(LogLevel::LogLevel_Info), newFormat.c_str(), args);
        va_end(args);
        return;
    }

    if (m_eLogLevel < LogLevel::LogLevel_Info)
    {
        return;
    }
    
    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
    va_end(args);

    m_oMutex.lock();
    printf("%s\n", sBuf);
    m_oMutex.unlock();
}

void Logger :: LogVerbose(const char * pcFormat, ...)
{
    string newFormat = "\033[45;37m " + string(pcFormat) + " \033[0m";

    if (m_pLogFunc != nullptr)
    {
        va_list args;
        va_start(args, pcFormat);
        m_pLogFunc(static_cast<int>(LogLevel::LogLevel_Verbose), newFormat.c_str(), args);
        va_end(args);
        return;
    }

    if (m_eLogLevel < LogLevel::LogLevel_Verbose)
    {
        return;
    }

    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
    va_end(args);

    m_oMutex.lock();
    printf("%s\n", sBuf);
    m_oMutex.unlock();
}

void Logger :: LogShowy(const char * pcFormat, ...)
{
    string newFormat = "\033[45;37m " + string(pcFormat) + " \033[0m";

    if (m_pLogFunc != nullptr)
    {
        va_list args;
        va_start(args, pcFormat);
        m_pLogFunc(static_cast<int>(LogLevel::LogLevel_Verbose), newFormat.c_str(), args);
        va_end(args);
        return;
    }

    if (m_eLogLevel < LogLevel::LogLevel_Verbose)
    {
        return;
    }
    
    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), newFormat.c_str(), args);
    va_end(args);

    m_oMutex.lock();
    printf("%s\n", sBuf);
    m_oMutex.unlock();
}

}


