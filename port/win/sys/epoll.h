#pragma once

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EPOLLIN         0x001
#define EPOLLPRI        0x002
#define EPOLLOUT        0x004
#define EPOLLRDNORM     0x040
#define EPOLLRDBAND     0x080
#define EPOLLWRNORM     0x100
#define EPOLLWRBAND     0x200
#define EPOLLMSG        0x400
#define EPOLLERR        0x008
#define EPOLLHUP        0x010
#define EPOLLRDHUP      0x2000
#define EPOLLWAKEUP     (1u << 29)
#define EPOLLONESHOT    (1u << 30)
#define EPOLLET         (1u << 31)

#define EPOLL_CTL_ADD 1 /* Add a file descriptor to the interface.  */
#define EPOLL_CTL_DEL 2 /* Remove a file descriptor from the interface.  */
#define EPOLL_CTL_MOD 3 /* Change file descriptor epoll_event structure.  */


typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      /* Epoll events */
    epoll_data_t data;        /* User data variable */
};

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_create(int size);
int epoll_wait(int epfd, struct epoll_event *events,
    int maxevents, int timeout);
int epoll_close(int epfd);

#ifdef __cplusplus
}
#endif
