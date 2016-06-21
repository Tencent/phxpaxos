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

#include "utils.h"

namespace phxkv
{

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

}

