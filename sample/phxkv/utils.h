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

#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <exception>
#include <cerrno>
#include <string.h>
#include <string>

namespace phxkv
{

class SysCallException : public std::exception {
public:
    SysCallException(int errCode, const std::string& errMsg, bool detail = true) : _errCode(errCode), _errMsg(errMsg) {
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
    std::string  _errMsg;
};


class SyncException : public SysCallException {
public:
    SyncException(int errCode, const std::string& errMsg, bool detail = true)
        : SysCallException(errCode, errMsg, detail) {}

    virtual ~SyncException() throw () {}
};

class Noncopyable {
protected:
    Noncopyable() {}
    ~Noncopyable() {}
private:
    Noncopyable(const Noncopyable&);
    const Noncopyable& operator=(const Noncopyable&);
};

class Mutex : public Noncopyable {
public:
    Mutex();

    ~Mutex();

    void lock();

    bool tryLock();

    void unlock();

private:

    friend class Condition;

    pthread_mutex_t _pm;
};

template <class Lock>
class ScopedLock : public Noncopyable {
public:
    ScopedLock(Lock& lock, bool locked = false) : _locked(locked), _lock(lock) {
        this->lock();
    }

    ~ScopedLock() {
        this->unlock();
    }

    void lock() {
        if (!_locked) {
            _lock.lock();
            _locked = true;
        }
    }

    void unlock() {
        if (_locked) {
            _lock.unlock();
            _locked = false;
        }
    }

private:
    bool _locked;
    Lock& _lock;
};


}
