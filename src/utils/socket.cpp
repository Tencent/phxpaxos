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

#include "socket.h"
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <iostream>
#include <sys/socket.h>
#include <netinet/tcp.h>

namespace phxpaxos {

using std::min;

///////////////////////////////////////////////////////////SocketAddress

SocketAddress::SocketAddress() {
    _addr.addr.sa_family = AF_INET;
    _addr.in.sin_port = 0;
    _addr.in.sin_addr.s_addr = htonl(INADDR_NONE);
}

SocketAddress::SocketAddress(unsigned short port) {
    setAddress(port);
}

SocketAddress::SocketAddress(const string& addr) {
    setAddress(addr);
}

SocketAddress::SocketAddress(const string& addr, unsigned short port) {
    setAddress(addr, port);
}

SocketAddress::SocketAddress(const Addr& addr) {
    setAddress(addr);
}

SocketAddress::SocketAddress(const sockaddr_in& addr) {
    setAddress(addr);
}

SocketAddress::SocketAddress(const sockaddr_un& addr) {
    setAddress(addr);
}

void SocketAddress::setAddress(unsigned short port) {
    _addr.addr.sa_family = AF_INET;
    _addr.in.sin_port = htons(port);
    _addr.in.sin_addr.s_addr = htonl(INADDR_ANY);
}

void SocketAddress::setAddress(const string& addr) {
    string::size_type pos = addr.find_last_of(":");
    if (pos == string::npos || pos == addr.size() - 1) {
        setAddress(addr, 0);
    } else {
        string port = addr.substr(pos + 1);
        setAddress(addr.substr(0, pos), (unsigned short)atoi(port.c_str()));
    }
}

void SocketAddress::setAddress(const string& addr, unsigned short port) {
    unsigned long ip = inet_addr(addr.c_str());
    if (ip == static_cast<unsigned long>(INADDR_NONE)) {
        throw SocketException("inet_addr error \"" + addr + "\"");
    }

    _addr.addr.sa_family = AF_INET;
    _addr.in.sin_port = htons(port);
    _addr.in.sin_addr.s_addr = ip;
}

void SocketAddress::setUnixDomain(const string& path) {
    if (path.size() + 1 > sizeof(_addr.un.sun_path)) {
        throw SocketException("unix domain path \"" + path + "\" too long");
    }

    _addr.addr.sa_family = AF_UNIX;
    strcpy(_addr.un.sun_path, path.c_str());
}

unsigned long SocketAddress::getIp() const {
    return _addr.in.sin_addr.s_addr;
}

unsigned short SocketAddress::getPort() const {
    return ntohs(_addr.in.sin_port);
}

void SocketAddress::getAddress(Addr& addr) const {
    memcpy(&addr, &_addr, sizeof(addr));
}

void SocketAddress::getAddress(sockaddr_in& addr) const {
    memcpy(&addr, &_addr.in, sizeof(addr));
}

void SocketAddress::getAddress(sockaddr_un& addr) const {
    memcpy(&addr, &_addr.un, sizeof(addr));
}

void SocketAddress::setAddress(const Addr& addr) {
    memcpy(&_addr, &addr, sizeof(addr));
}

void SocketAddress::setAddress(const sockaddr_in& addr) {
    memcpy(&_addr.in, &addr, sizeof(addr));
    _addr.addr.sa_family = AF_INET;
}

void SocketAddress::setAddress(const sockaddr_un& addr) {
    memcpy(&_addr.un, &addr, sizeof(addr));
}

int SocketAddress::getFamily() const {
    return _addr.addr.sa_family;
}

socklen_t SocketAddress::getAddressLength(const Addr& addr) {
    if (addr.addr.sa_family == AF_INET) {
        return sizeof(addr.in);
    } else if (addr.addr.sa_family == AF_UNIX || addr.addr.sa_family == AF_LOCAL) {
        return sizeof(addr.un);
    }
    return sizeof(addr);
}

string SocketAddress::getHost() const {
    if (_addr.addr.sa_family == AF_UNIX || _addr.addr.sa_family == AF_LOCAL) {
        return _addr.un.sun_path;
    } else {
        char buff[16];

        if (inet_ntop(AF_INET, &_addr.in.sin_addr, buff, sizeof(buff)) == 0) {
            return string();
        } else {
            return buff;
        }
    }
}

string SocketAddress::toString() const {
    if (_addr.addr.sa_family == AF_UNIX || _addr.addr.sa_family == AF_LOCAL) {
        return _addr.un.sun_path;
    }
    return getHost() + ":" + str(ntohs(_addr.in.sin_port));
}

bool SocketAddress::operator ==(const SocketAddress& addr) const {
    if (_addr.addr.sa_family != addr._addr.addr.sa_family) {
        return false;
    } else if (_addr.addr.sa_family == AF_UNIX || _addr.addr.sa_family == AF_LOCAL) {
        return strcmp(_addr.un.sun_path, addr._addr.un.sun_path) == 0;
    } else {
        return _addr.in.sin_addr.s_addr == addr._addr.in.sin_addr.s_addr
                && _addr.in.sin_port == addr._addr.in.sin_port;
    }
}

///////////////////////////////////////////////////////////SocketBase

SocketBase::SocketBase() : _family(AF_INET), _handle(-1) {}

SocketBase::SocketBase(int family, int handle) : _family(family), _handle(handle) {
    if (handle == -1) {
        initHandle(family);
    }
}

SocketBase::~SocketBase() {
    if (_handle != -1) {
        ::close(_handle);
        _handle = -1;
    }
}

void SocketBase::initHandle(int family) {
    _handle = ::socket(family, SOCK_STREAM, 0);
    if (_handle == -1) {
        throw SocketException("socket error");
    }
}

int SocketBase::getFamily() const {
    return _family;
}

int SocketBase::getSocketHandle() const {
    return _handle;
}

void SocketBase::setSocketHandle(int handle, int family) {
    if (_handle != handle) {
        close();

        _handle = handle;
        _family = family;
    }
}

int SocketBase::detachSocketHandle() {
    int handle = _handle;
    _handle = -1;
    _family = AF_INET;
    return handle;
}

bool SocketBase::getNonBlocking() const {
    return getNonBlocking(_handle);
}

void SocketBase::setNonBlocking(bool on) {
    setNonBlocking(_handle, on);
}

bool SocketBase::getNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return flags & O_NONBLOCK;
}

void SocketBase::setNonBlocking(int fd, bool on) {
    int flags = ::fcntl(fd, F_GETFL, 0);

    if (on) {
        if (flags & O_NONBLOCK) {
            return;
        }
        flags |= O_NONBLOCK;
    } else {
        if (!(flags & O_NONBLOCK)) {
            return;
        }
        flags &= ~O_NONBLOCK;
    }

    if (0 != ::fcntl(fd, F_SETFL, flags)) {
        ;
    }
}

socklen_t SocketBase::getOption(int level, int option, void* value, socklen_t optLen) const {
    if (::getsockopt(_handle, level, option, static_cast<char*>(value), &optLen) == -1) {
        throw SocketException("getsockopt error");
    }
    return optLen;
}

void SocketBase::setOption(int level, int option, void* value, socklen_t optLen) const {
    if (::setsockopt(_handle, level, option, static_cast<char*>(value), optLen) == -1) {
        throw SocketException("setsockopt error");
    }
}

void SocketBase::close() {
    if (_handle != -1) {
        if (::close(_handle) == -1) {
            _handle = -1;
            throw SocketException("close error");
        } else {
            _handle = -1;
        }
    }
}

void SocketBase::reset() {
    close();
    initHandle(_family);
}

///////////////////////////////////////////////////////////Socket

const static int SOCKET_BUF_SIZE = 512;

Socket::Socket() : SocketBase(AF_INET, -1) {}

Socket::Socket(int handle) : SocketBase(AF_INET, handle)  {}

Socket::Socket(const SocketAddress& addr) {
    connect(addr);
}

Socket::Socket(int family, int handle) : SocketBase(family, handle) {}

Socket::~Socket() {}

int Socket::getSendTimeout() const {
    timeval tv;
    getOption(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(timeval));
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void Socket::setSendTimeout(int timeout) {
    if (timeout >= 0) {
        timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        setOption(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    } else {
        setOption(SOL_SOCKET, SO_SNDTIMEO, 0, sizeof(int));
    }
}

int Socket::getReceiveTimeout() const {
    timeval tv;
    getOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval));
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
    
void Socket::setReceiveTimeout(int timeout) {
    if (timeout >= 0) {
        timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        setOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    } else {
        setOption(SOL_SOCKET, SO_RCVTIMEO, 0, sizeof(int));
    }
}

void Socket::setSendBufferSize(int size) {
    setOption(SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

void Socket::setReceiveBufferSize(int size) {
    setOption(SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

void Socket::setQuickAck(bool on) {
    int v = on ? 1 : 0;
    setOption(IPPROTO_TCP, TCP_QUICKACK, &v, sizeof(v));
}

void Socket::setNoDelay(bool on) {
    int v = on ? 1 : 0;
    setOption(IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
}

SocketAddress Socket::getRemoteAddress() const {
    return getRemoteAddress(_handle);
}
    
SocketAddress Socket::getRemoteAddress(int fd) {
    SocketAddress::Addr addr;
    socklen_t len = sizeof(addr);

    phxpaxos::SocketAddress remoteAddr;
    if (::getpeername(fd, &addr.addr, &len) == 0) {
        remoteAddr.setAddress(addr);
    }

    return remoteAddr;
}

SocketAddress Socket::getLocalAddress() const {
    return getLocalAddress(_handle);
}

SocketAddress Socket::getLocalAddress(int fd) {
    SocketAddress::Addr addr;
    socklen_t len = sizeof(addr);

    phxpaxos::SocketAddress localAddr;
    if (::getsockname(fd, &addr.addr, &len) == 0) {
        localAddr.setAddress(addr);
    }

    return localAddr;
}

void Socket::connect(const SocketAddress& addr) {
    SocketAddress::Addr sockAddr;
    addr.getAddress(sockAddr);

    if (_handle == -1 || _family != addr.getFamily()) {
        close();
        initHandle(addr.getFamily());
    }

    if (_handle < 0) {
        throw SocketException("bad handle");
    }

    if (::connect(_handle, &sockAddr.addr, SocketAddress::getAddressLength(sockAddr)) == -1) {
        if (errno != EINPROGRESS) {
            string msg = "connect " + addr.toString() + " error";
            throw SocketException(msg);
        } else if (!getNonBlocking()) {
            string msg = "connect " + addr.toString() + " timeout";
            throw SocketException(msg);
        }
    }
}

int Socket::send(const char* data, int dataSize, bool* again) {
    const char* p = data;
    int n = 0;

    if (again) {
        *again = false;
    }

    while (dataSize > 0) {
        n = ::send(_handle, p, dataSize, 0);
        if (n > 0) {
            p += n;
            dataSize -= n;
        } else if (errno == EAGAIN) { 
            if (again) {
                *again = true;
            }

            if (!getNonBlocking()) {
                throw SocketException("send timeout");
            }

            break;
        } else if (errno == EINTR) { 
            continue;
        } else {
            throw SocketException("send error");
        }
    }

    return p - data;
}

int Socket::receive(char* buffer, int bufferSize, bool* again) {
    char* p = buffer;
    int n = 0;

    if (again) {
        *again = false;
    }

    while (bufferSize > 0) {
        n = ::recv(_handle, p, bufferSize, 0);
        if (n > 0) {
            p += n;

            if (n < bufferSize) {
                break;
            }

            bufferSize -= n;
        } else if (n == 0) { 
            break;
        } else if (errno == EAGAIN) { 
            if (again) {
                *again = true;
            }

            if (!getNonBlocking()) {
                throw SocketException("recv timeout");
            }

            break;
        } else if (errno == EINTR) {
            continue;
        } else {
            throw SocketException("recv error");
        }
    }

    return p - buffer;
}

void Socket::shutdownInput() {
    if (::shutdown(_handle, SHUT_RD) == -1) {
        throw SocketException("shutdown(SHUT_RD) error");
    }
}

void Socket::shutdownOutput() {
    if (::shutdown(_handle, SHUT_WR) == -1) {
        throw SocketException("shutdown(SHUT_WR) error");
    }
}

void Socket::shutdown() {
    if (::shutdown(_handle, SHUT_RDWR) == -1) {
        throw SocketException("shutdown error");
    }
}

///////////////////////////////////////////////////////////ServerSocket

ServerSocket::ServerSocket() {}

ServerSocket::ServerSocket(const SocketAddress& addr) : SocketBase(addr.getFamily(), -1) {
    listen(addr);
}

ServerSocket::~ServerSocket() {}

void ServerSocket::listen(const SocketAddress& addr, int backlog) {
    SocketAddress::Addr localAddr;
    addr.getAddress(localAddr);

    if (_handle == -1 || _family != addr.getFamily()) {
        close();
        initHandle(addr.getFamily());
    }

    if (_handle < 0) {
        throw SocketException("bad handle");
    }

    if (addr.getFamily() == AF_UNIX) {
        ::unlink(localAddr.un.sun_path);
    } else {
        int reuse = 1;
        setOption(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    }

    if (::bind(_handle, &localAddr.addr, SocketAddress::getAddressLength(localAddr)) == -1) {
        throw SocketException("bind error");
    }

    if (::listen(_handle, backlog) == -1) {
        throw SocketException("listen error");
    }
}

int ServerSocket::getAcceptTimeout() const {
    timeval tv;
    getOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval));
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void ServerSocket::setAcceptTimeout(int timeout) {
    timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    setOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

Socket* ServerSocket::accept() {
    int fd = acceptfd(NULL);
    if (fd >= 0) {
        return new Socket(_family, fd);
    } else {
        return NULL;
    }
}

int ServerSocket::acceptfd(SocketAddress* addr) {
    SocketAddress::Addr a;
    socklen_t n = sizeof(a);

    int fd = ::accept(_handle, &a.addr, &n);
    if (fd == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
        throw SocketException("accept error");
    } else if (fd >= 0 && addr) {
        addr->setAddress(a);
    }
    return fd;
}

}


