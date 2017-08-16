#pragma once

#include <winsock2.h>

#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH

extern "C" int socketpair(SOCKET socks[2]);
