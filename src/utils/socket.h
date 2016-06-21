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

#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string>
#include "util.h"

namespace phxpaxos {

using std::string;

//////////////////////////////////////////////////////////////////SocketAddress

class SocketAddress {
public:
    enum Type {
        TYPE_LOOPBACK = 1, 
        TYPE_INNER = 2, 
        TYPE_OUTER = 3 
    };

    union Addr {
        sockaddr addr;
        sockaddr_in in;
        sockaddr_un un;
    };

    SocketAddress();

    SocketAddress(unsigned short port);

    SocketAddress(const string& addr);

    SocketAddress(const string& addr, unsigned short port);

    SocketAddress(const Addr& addr);

    SocketAddress(const sockaddr_in& addr);

    SocketAddress(const sockaddr_un& addr);

    void setAddress(unsigned short port);

    void setAddress(const string& addr);

    void setAddress(const string& addr, unsigned short port);

    void setUnixDomain(const string& path);

    unsigned long getIp() const;

    unsigned short getPort() const;

    void getAddress(Addr& addr) const;

    void getAddress(sockaddr_in& addr) const;

    void getAddress(sockaddr_un& addr) const;

    void setAddress(const Addr& addr);

    void setAddress(const sockaddr_in& addr);

    void setAddress(const sockaddr_un& addr);

    string getHost() const;

    string toString() const;

    int getFamily() const;

    static socklen_t getAddressLength(const Addr& addr);

    bool operator ==(const SocketAddress& addr) const;

    static Type getAddressType(const string& ip);

private:

    Addr _addr;
};

///////////////////////////////////////////////////////////////////////Socket

class SocketBase {
public:
    SocketBase();

    SocketBase(int family, int handle);

    virtual ~SocketBase();

    virtual int getFamily() const;

    virtual int getSocketHandle() const;

    virtual void setSocketHandle(int handle, int family = AF_INET);

    virtual int detachSocketHandle();

    virtual bool getNonBlocking() const;

    virtual void setNonBlocking(bool on);

    static void setNonBlocking(int fd, bool on);

    static bool getNonBlocking(int fd);

    virtual void close();

    virtual void reset();

protected:

    void initHandle(int family);

    socklen_t getOption(int level, int option, void* value, socklen_t optLen) const;

    void setOption(int level, int option, void* value, socklen_t optLen) const;

    int _family;
    int _handle;
};

class Socket : public SocketBase {
public:
    Socket();

    Socket(const SocketAddress& addr);

    Socket(int handle);

    Socket(int family, int handle);

    virtual ~Socket();

    int getSendTimeout() const;

    void setSendTimeout(int timeout);

    int getReceiveTimeout() const;

    void setReceiveTimeout(int timeout);

    void setSendBufferSize(int size);

    void setReceiveBufferSize(int size);

    void setQuickAck(bool on);

    void setNoDelay(bool on);

    SocketAddress getRemoteAddress() const;

    static SocketAddress getRemoteAddress(int fd);
    
    SocketAddress getLocalAddress() const;

    static SocketAddress getLocalAddress(int fd);

    virtual void connect(const SocketAddress& addr);

    virtual int send(const char* data, int dataSize, bool* again = 0);

    virtual int receive(char* buffer, int bufferSize, bool* again = 0);

    virtual void shutdownInput();

    virtual void shutdownOutput();

    virtual void shutdown();
};

///////////////////////////////////////////////////////////////////////ServerSocket

class ServerSocket : public SocketBase {
public:
    ServerSocket();

    ServerSocket(const SocketAddress& addr);

    virtual ~ServerSocket();

    int getAcceptTimeout() const;

    void setAcceptTimeout(int timeout);

    virtual void listen(const SocketAddress& addr, int backlog = SOMAXCONN);

    virtual Socket* accept();

    virtual int acceptfd(SocketAddress* addr);
};

///////////////////////////////////////////////////////////////////////SocketException

class SocketException : public SysCallException {
public:
    SocketException(const string& errMsg, bool detail = true)
        : SysCallException(errno, errMsg, detail) {}

    virtual ~SocketException() throw () {}
};

} 

