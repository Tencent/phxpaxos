#include "notify.h"
#include "commdef.h"
#include <WinSock2.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace phxpaxos
{
Notify :: Notify(EventLoop * poEventLoop)
    : Event(poEventLoop)
{
    m_iPipeFD[0] = -1;
    m_iPipeFD[1] = -1;
    m_sHost = "Notify";
}

Notify :: ~Notify()
{
    for (int i = 0; i < 2; i++)
    {
        if (m_iPipeFD[i] != -1)
        {
            ::closesocket(m_iPipeFD[i]);
        }
    }
}

int Notify :: Init()
{
    int ret = socketpair(m_iPipeFD);
    if (ret != 0)
    {
        PLErr("create pipe fail, ret %d", ret);
        return ret;
    }
    u_long v = 1;
    ioctlsocket(m_iPipeFD[0], FIONBIO, &v);
    ioctlsocket(m_iPipeFD[1], FIONBIO, &v);

    AddEvent(EPOLLIN);
    return 0;
}

int Notify :: GetSocketFd() const
{
    return m_iPipeFD[0];
}

const std::string & Notify :: GetSocketHost()
{
    return m_sHost;
}

void Notify :: SendNotify()
{
    ssize_t iWriteLen = ::send(m_iPipeFD[1], "a", 1, 0);
    if (iWriteLen != 1)
    {
        //PLErr("notify error, writelen %d", iWriteLen);
    }
}

int Notify :: OnRead()
{
    char sTmp[2] = {0};
    int iReadLen = ::recv(m_iPipeFD[0], sTmp, 1, 0);
    if (iReadLen < 0)
    {
        return -1;
    }

    return 0;
}

void Notify :: OnError(bool & bNeedDelete)
{
    bNeedDelete = false;
}

}


