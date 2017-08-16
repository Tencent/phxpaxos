#include "poll.h"

extern "C" int poll(struct pollfd *fds, unsigned int nfds, int timeout)
{
    unsigned int max_fd = 0;
    fd_set read_fds, write_fds, except_fds;
    struct timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };

    for (unsigned int i = 0; i < nfds; ++i)
        if (fds[i].fd > max_fd)
            max_fd = fds[i].fd;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    for (unsigned int i = 0; i < nfds; ++i)
    {
        fds[i].revents = 0;
        if (fds[i].events & POLLIN)
            FD_SET(fds[i].fd, &read_fds);
        if (fds[i].events & POLLOUT)
            FD_SET(fds[i].fd, &write_fds);
        if (fds[i].events & POLLPRI)
            FD_SET(fds[i].fd, &except_fds);
    }


    int ret = 0;
    ret = select (max_fd + 1, &read_fds, &write_fds, &except_fds, timeout < 0 ? NULL : &tv);

    for (unsigned int i = 0; i < nfds; ++i)
    {
        if (ret < 0)
            fds[i].revents = POLLERR;
        else
        {
            fds[i].revents = 0;
            if (FD_ISSET (fds[i].fd, &read_fds))
                fds[i].revents |= POLLIN;
            if (FD_ISSET (fds[i].fd, &write_fds))
                fds[i].revents |= POLLOUT;
            if (FD_ISSET (fds[i].fd, &except_fds))
                fds[i].revents |= POLLPRI;
        }
    }

    return ret;
}
