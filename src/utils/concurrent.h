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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <semaphore.h>

namespace phxpaxos {

using std::deque;

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

class Condition : public Noncopyable {
public:
    Condition(Mutex& mutex);

    ~Condition();

    void signal();

    void broadcast();

    void wait();

    bool tryWait(int ms);

private:
    bool tryWait(const timespec* timeout);

    Mutex& _mutex;
    pthread_cond_t _pc;
    pthread_condattr_t _pc_attr;
};

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
class Queue : public Noncopyable {
public:
    Queue() : _cond(_mutex), _size(0) {}

    virtual ~Queue() {}

    T& peek() {
        while (empty()) {
            _cond.wait();
        }
        return _storage.front();
    }

    size_t peek(T& value) {
        while (empty()) {
            _cond.wait();
        }
        value = _storage.front();
        return _size;
    }

    bool peek(T& t, int timeoutMS) {
        while (empty()) {
            if (!_cond.tryWait(timeoutMS)) {
                return false;
            }
        }
        t = _storage.front();
        return true;
    }

    size_t pop(T* values, size_t n) {
        while (empty()) {
            _cond.wait();
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
            _cond.signal();
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
        _cond.signal();
    }

    void broadcast() {
        _cond.broadcast();
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
    Mutex _mutex;
    Condition _cond;
    deque<T> _storage;
    size_t _size;
};

template <class T, class C = std::less<T>, class S = std::vector<T> >
class Heap : public Noncopyable {
public:
    typedef S Storage;
    typedef typename Storage::const_iterator const_iterator;
    typedef typename Storage::iterator iterator;

    void push(const T& data) {
        _storage.push_back(data);
        std::push_heap(_storage.begin(), _storage.end(), C());
    }

    T& peek() {
        return _storage.front();
    }

    void pop() {
        std::pop_heap(_storage.begin(), _storage.end(), C());
        _storage.pop_back();
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

    const_iterator begin() const {
        return _storage.begin();
    }

    iterator begin() {
        return _storage.begin();
    }

    const_iterator end() const {
       return _storage.end();
    }

    iterator end() {
       return _storage.end();
    }

    void lock() {
        _mutex.lock();
    }

    void unlock() {
        _mutex.unlock();
    }

private:
    Storage _storage;
    phxpaxos::Mutex _mutex;
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

