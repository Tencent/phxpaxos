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

#include "util.h"
#include <deque>
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue>
#include <list>
#include <map>
#include <iostream>
#include <string>
#include <condition_variable>
#include <mutex>

namespace phxpaxos {

using std::deque;

class ThreadAttr {
public:
    ThreadAttr();

    ~ThreadAttr();

    void setScope(bool sys);

    void setStackSize(size_t n);

    void setDetached(bool detached);

    void setPriority(int prio);
    
    pthread_attr_t* impl();

private:

    pthread_attr_t _attr;
};

class Thread : public Noncopyable {
public:
    Thread();

    virtual ~Thread();

    void start();

    void start(ThreadAttr& attr);

    void join();

    void detach();
    
    pthread_t getId() const;

    void kill(int sig);

    virtual void run() = 0;

    static void sleep(int ms);

protected:
    pthread_t _thread;
};

template <class T>
class Queue {
public:
    Queue() : _lock(_mutex), _size(0) { _lock.unlock(); }

    virtual ~Queue() {}

    T& peek() {
        while (empty()) {
            _cond.wait(_lock);
        }
        return _storage.front();
    }

    size_t peek(T& value) {
        while (empty()) {
            _cond.wait(_lock);
        }
        value = _storage.front();
        return _size;
    }

    bool peek(T& t, int timeoutMS) {
        while (empty()) {
            if (_cond.wait_for(_lock, std::chrono::milliseconds(timeoutMS)) == std::cv_status::timeout) {
                return false;
            }
        }
        t = _storage.front();
        return true;
    }

    size_t pop(T* values, size_t n) {
        while (empty()) {
            _cond.wait(_lock);
        }

        size_t i = 0;
        while (!_storage.empty() && i < n) {
            values[i] = _storage.front();
            _storage.pop_front();
            --_size;
            ++i;
        }

        return i;
    }

    size_t pop() {
        _storage.pop_front();
        return --_size;
    }

    virtual size_t add(const T& t, bool signal = true, bool back = true) {
        if (back) {
            _storage.push_back(t);
        } else {
            _storage.push_front(t);
        }

        if (signal) {
            _cond.notify_one();
        }

        return ++_size;
    }

    bool empty() const {
        return _storage.empty();
    }

    size_t size() const {
        return _storage.size();
    }

    void clear() {
        _storage.clear();
    }

    void signal() {
        _cond.notify_one();
    }

    void broadcast() {
        _cond.notify_all();
    }

    virtual void lock() {
        _mutex.lock();
    }

    virtual void unlock() {
        _mutex.unlock();
    }

    void swap(Queue& q) {
        _storage.swap( q._storage );
        int size = q._size;
        q._size = _size;
        _size = size;
    }

protected:
    std::mutex _mutex;
    std::unique_lock<std::mutex> _lock;
    std::condition_variable _cond;
    deque<T> _storage;
    size_t _size;
};

class SyncException : public SysCallException {
public:
    SyncException(int errCode, const string& errMsg, bool detail = true)
        : SysCallException(errCode, errMsg, detail) {}

    virtual ~SyncException() throw () {}
};

class ThreadException : public SysCallException {
public:
    ThreadException(const string& errMsg, bool detail = true)
        : SysCallException(errno, errMsg, detail) {}

    virtual ~ThreadException() throw () {}
};

} 

