#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <io.h>


#define F_OK 0

typedef long ssize_t;

#define snprintf _snprintf


#ifdef __cplusplus
extern "C" {
#endif
errno_t rand_r(unsigned int* v);
#define strtoll  _strtoi64
#define strtoull _strtoui64


int WinsockInit();
void WinsockShutdown();

#ifdef __cplusplus
}
#endif
