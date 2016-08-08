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

#include <exception>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <inttypes.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h> 
#include <stdio.h>
#include <string.h>
#include <chrono>

namespace phxpaxos {

using std::string;
using std::exception;
using std::ostringstream;

template <class T>
string str(const T& t) { 
    ostringstream os;
    os << t;
    return os.str();
}

class SysCallException : public exception {
public:
    SysCallException(int errCode, const string& errMsg, bool detail = true) : _errCode(errCode), _errMsg(errMsg) {
        if (detail) {
            _errMsg.append(", ").append(::strerror(errCode));
        }
    }

    virtual ~SysCallException() throw () {}

    int getErrorCode() const throw () {
        return _errCode;
    }

    const char* what() const throw () {
        return _errMsg.c_str();
    }

protected:
    int     _errCode;
    string  _errMsg;
};

class Noncopyable {
protected:
    Noncopyable() {}
    ~Noncopyable() {}
private:
    Noncopyable(const Noncopyable&);
    const Noncopyable& operator=(const Noncopyable&);
};

///////////////////////////////////////////////////////////////

class Time
{
public:
    static const uint64_t GetTimestampMS();

    static const uint64_t GetSteadyClockMS();

    static void MsSleep(const int iTimeMs);
};

class FileUtils
{
public:
    static int IsDir(const std::string & sPath, bool & bIsDir);

    static int DeleteDir(const std::string & sDirPath);

    static int IterDir(const std::string & sDirPath, std::vector<std::string> & vecFilePathList);
};

class TimeStat
{
public:
    TimeStat();

    int Point();
private:
    uint64_t m_llTime;
};

class OtherUtils
{
public:
    static uint64_t GenGid(const uint64_t llNodeID);

    static const uint32_t FastRand();
};


} 
