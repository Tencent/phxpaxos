#include "epoll.h"
#include <vector>
#include <mutex>
#include <map>
#include <assert.h>
#include <WinSock2.h>

#define Log(fmt, ...) printf("%s(%d), %s" fmt "\n", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


class EpollCtx
{
public:
    EpollCtx(int /*size*/) {}
    ~EpollCtx() {}

    int Ctl(int op, int fd, struct epoll_event* event);
    int Wait(struct epoll_event* events, int maxevents, int timeout);

private:
    int Add(int fd, struct epoll_event* event);
    int Mod(int fd, struct epoll_event* event);
    int Del(int fd, struct epoll_event* event);
    std::mutex m_mutex;
    std::map<int, epoll_event> m_eventMap;
};

int EpollCtx::Ctl(int op, int fd, struct epoll_event* event)
{
    assert((event->events & ~(EPOLLIN|EPOLLOUT|EPOLLERR)) == 0);

    if (op == EPOLL_CTL_ADD)
    {
        return Add(fd, event);
    }
    else if (op == EPOLL_CTL_DEL)
    {
        return Del(fd, event);
    }
    else if (op == EPOLL_CTL_MOD)
    {
        return Mod(fd, event);
    }
    else
    {
        return -1;
    }
}


int EpollCtx::Wait(struct epoll_event* events, int maxevents, int timeout)
{
    fd_set read_fds, write_fds, except_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    int max_fd = 0;
    {
        if (maxevents < m_eventMap.size())
        {
            Log("Events may exceed parameter");
        }

        for (auto it = m_eventMap.begin(); it != m_eventMap.end(); ++it)
        {
            if (it->second.events & EPOLLIN)
            {
                FD_SET(it->first, &read_fds);
            }
            if (it->second.events & EPOLLOUT)
            {
                FD_SET(it->first, &write_fds);
            }

            FD_SET(it->first, &except_fds);
            if (it->first > max_fd)
            {
                max_fd = it->first;
            }
        }
    }

    timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
    int ret = 0;
/*
    if (max_fd == 0)
    {
        Sleep(timeout);
        return 0;
    }
*/
    ret = select(max_fd + 1, &read_fds, &write_fds, &except_fds, timeout < 0 ? NULL : &tv);
    if (ret <= 0)
    {
        return ret;
    }
    int i = 0;
    for (auto it = m_eventMap.begin(); it != m_eventMap.end(); ++it)
    {
        if (FD_ISSET(it->first, &read_fds))
        {
            events[i].events |= EPOLLIN;
        }
        if (FD_ISSET(it->first, &write_fds))
        {
            events[i].events |= EPOLLOUT;
        }
        if (FD_ISSET(it->first, &except_fds))
        {
            events[i].events |= EPOLLERR;
        }

        if (events[i].events)
        {
            events[i++].data = it->second.data;
        }
    }


    return i;
}


int EpollCtx::Add(int fd, struct epoll_event* event)
{
    if (m_eventMap.find(fd) != m_eventMap.end())
    {
        Log("fd %d already in epoll", fd);
        return -1;
    }

    m_eventMap.insert(std::make_pair(fd, *event));

    return 0;
}


int EpollCtx::Mod(int fd, struct epoll_event* event)
{
    auto it = m_eventMap.find(fd);
    if (it == m_eventMap.end())
    {
        Log("fd %d not in epoll", fd);
        return -1;
    }

    it->second = *event;
    return 0;
}


int EpollCtx::Del(int fd, struct epoll_event* event)
{
    auto it = m_eventMap.find(fd);
    if (it == m_eventMap.end())
    {
        Log("fd %d not in epoll", fd);
        return -1;
    }

    m_eventMap.erase(it);
    return 0;
}

static const int kEpollCtxSize = 100;
static EpollCtx* gEpollCtx[kEpollCtxSize];

extern "C" int epoll_create(int size)
{
    for (int i = 0; i < kEpollCtxSize; ++i)
    {
        if (gEpollCtx[i] == nullptr)
        {
            EpollCtx* ctx = new EpollCtx(size);
            gEpollCtx[i] = ctx;
            return i;
        }
    }
    Log("Too many epoll fd");
    return -1;
}

extern "C" int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    if (event == nullptr)
    {
        Log("event is NULL");
        return -1;
    }
    EpollCtx* ctx = nullptr;
    {
        if (gEpollCtx[epfd] == nullptr)
        {
            return -1;
        }
        ctx = gEpollCtx[epfd];
    }
    ctx->Ctl(op, fd, event);
    return 0;
}

extern "C" int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    if (events == nullptr)
    {
        Log("events is NULL");
        return -1;
    }
    EpollCtx* ctx = nullptr;
    {
        if (gEpollCtx[epfd] == nullptr)
        {
            Log("epfd not a epoll fd: %d", epfd);
            return -1;
        }
        ctx = gEpollCtx[epfd];
    }
    memset(events, 0, sizeof(epoll_event) * maxevents);
    return ctx->Wait(events, maxevents, timeout);
}

extern "C" int epoll_close(int epfd)
{
    if (epfd < 0 || epfd > kEpollCtxSize || gEpollCtx[epfd] == nullptr)
    {
        return -1;
    }
    EpollCtx* ctx = gEpollCtx[epfd];
    delete ctx;
    gEpollCtx[epfd] = nullptr;
    return 0;
}

