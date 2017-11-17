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
#include <functional>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static void* mmThreadRun(void* p) {
    phxpaxos::Thread* thread = (phxpaxos::Thread*)p;
    thread->run();
    return 0;
}

namespace phxpaxos {

///////////////////////////////////////////////////////////Thread

Thread::Thread() {}

Thread::~Thread() {}

void Thread::start() {
    _thread = std::thread(std::bind(&mmThreadRun, this));
}

void Thread::join() {
    _thread.join();
}

void Thread::detach() {
    _thread.detach();
}
    
std::thread::id Thread::getId() const {
    return _thread.get_id();
}

void Thread::sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} 


