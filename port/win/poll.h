#pragma once

#include <winsock2.h>

#ifdef __cplusplus
extern "C" {
#endif

int poll(struct pollfd *fds, unsigned int nfds, int timeout);

#ifdef __cplusplus
}
#endif
