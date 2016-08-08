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

#include "concurrent.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static void* mmThreadRun(void* p) {
    phxpaxos::Thread* thread = (phxpaxos::Thread*)p;
    thread->run();
    return 0;
}

namespace phxpaxos {

////////////////////////////////////////////////////////////////Mutex

Mutex::Mutex() {
    if (pthread_mutex_init(&_pm, 0)) {
        throw SyncException(errno, "pthread_mutex_init error");
    }
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&_pm);
}

void Mutex::lock() {
    int ret = pthread_mutex_lock(&_pm);
    if (ret) {
        throw SyncException(ret, "pthread_mutex_lock error");
    }
}

bool Mutex::tryLock() {
    int ret = pthread_mutex_trylock(&_pm);
    if (ret) {
        if (ret == EBUSY) {
            return false;
        }
        throw SyncException(ret, "pthread_mutex_trylock error");
    }
    return true;
}

void Mutex::unlock() {
    int ret = pthread_mutex_unlock(&_pm);
    if (ret) {
        throw SyncException(ret, "pthread_mutex_unlock error");
    }
}

///////////////////////////////////////////////////////////Condition

Condition::Condition(Mutex& mutex) : _mutex(mutex) {
    if (pthread_condattr_init(&_pc_attr)) {
        throw SyncException(errno, "pthread_condattr_init error");
    }

    pthread_condattr_setclock(&_pc_attr, CLOCK_MONOTONIC);

    if (pthread_cond_init(&_pc, &_pc_attr)) {
        throw SyncException(errno, "pthread_cond_init error");
    }
}

Condition::~Condition() {
    pthread_cond_destroy(&_pc);
}

void Condition::signal() {
    if (pthread_cond_signal(&_pc)) {
        throw SyncException(errno, "pthread_cond_signal error");
    }
}

void Condition::broadcast() {
    if (pthread_cond_broadcast(&_pc)) {
        throw SyncException(errno, "pthread_cond_broadcast error");
    }
}

void Condition::wait() {
    if (pthread_cond_wait(&_pc, &_mutex._pm)) {
        throw SyncException(errno, "pthread_cond_wait error");
    }
}

bool Condition::tryWait(int ms) {
    uint64_t timeout = Time::GetSteadyClockMS() + ms;

    timespec t;
    t.tv_sec = (time_t)(timeout / 1000);
    t.tv_nsec = (timeout % 1000) * 1000000;

    return tryWait(&t);
}

bool Condition::tryWait(const timespec* timeout) {
    int ret = pthread_cond_timedwait(&_pc, &_mutex._pm, timeout);
    if (ret) {
        if (ret == ETIMEDOUT) {
            return false;
        }
        throw SyncException(errno, "pthread_cond_timedwait error");
    }
    return true;
}


///////////////////////////////////////////////////////////ThreadAttr

ThreadAttr::ThreadAttr() {
    if (pthread_attr_init(&_attr) != 0) {
        throw ThreadException("pthread_attr_init error");
    }
}

ThreadAttr::~ThreadAttr() {
    pthread_attr_destroy(&_attr);
}

void ThreadAttr::setScope(bool sys) {
    int scope = sys ? PTHREAD_SCOPE_SYSTEM : PTHREAD_SCOPE_PROCESS;
    pthread_attr_setscope(&_attr, scope);
}

void ThreadAttr::setStackSize(size_t n) {
    pthread_attr_setstacksize(&_attr, n);
}

void ThreadAttr::setDetached(bool detached) {
    int state = detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;
    pthread_attr_setdetachstate(&_attr, state);
}

void ThreadAttr::setPriority(int prio) {
    sched_param param;

    pthread_attr_getschedparam(&_attr, &param);
    param.sched_priority = prio;

    pthread_attr_setschedparam(&_attr, &param);
}

pthread_attr_t* ThreadAttr::impl() {
    return &_attr;
}

///////////////////////////////////////////////////////////Thread

Thread::Thread() : _thread(0) {}

Thread::~Thread() {}

void Thread::start() {
    int rc = pthread_create(&_thread, 0, mmThreadRun, this);
    if (rc != 0) {
        throw ThreadException("pthread_create error");
    }
}

void Thread::start(ThreadAttr& attr) {
    int rc = pthread_create(&_thread, attr.impl(), mmThreadRun, this);
    if (rc != 0) {
        throw ThreadException("pthread_create error");
    }
}

void Thread::join() {
    int rc = pthread_join(_thread, 0);
    if (rc != 0) {
        throw ThreadException("pthread_join error");
    }
}

void Thread::detach() {
    int rc = pthread_detach(_thread);
    if (rc != 0) {
        throw ThreadException("pthread_detach error");
    }
}
    
pthread_t Thread::getId() const {
    return _thread;
}

void Thread::kill(int sig) {
    int rc = pthread_kill(_thread, sig);
    if (rc != 0) {
        throw ThreadException("pthread_kill error");
    }
}

void Thread::sleep(int ms) {
    timespec t;
    t.tv_sec = ms / 1000;
    t.tv_nsec = (ms % 1000) * 1000000;

    int ret = 0;
    do {
        ret = ::nanosleep(&t, &t);
    } while (ret == -1 && errno == EINTR);
}

} 


