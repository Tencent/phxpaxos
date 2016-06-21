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

#include "commdef.h"

namespace phxpaxos
{

class EventLoop;

class Event
{
public:
    Event(EventLoop * poEventLoop);
    virtual ~Event();

    virtual int GetSocketFd() const = 0;

    virtual const std::string & GetSocketHost() = 0;

    virtual int OnRead();

    virtual int OnWrite();

    virtual void OnError(bool & bNeedDelete) = 0;

    virtual void OnTimeout(const uint32_t iTimerID, const int iType);

public:
    void AddEvent(const int iEvents);
    
    void RemoveEvent(const int iEvents);

    void JumpoutEpollWait();

    const bool IsDestroy() const;

    void Destroy();

public:
    void AddTimer(const int iTimeoutMs, const int iType, uint32_t & iTimerID);

    void RemoveTimer(const uint32_t iTimerID);

protected:
    int m_iEvents;
    EventLoop * m_poEventLoop;

    bool m_bIsDestroy;
};
    
}
