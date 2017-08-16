#define _CRT_RAND_S
#include <stdlib.h>
#include <stdio.h>
#include "port_win.h"

#ifdef __cplusplus
extern "C" {
#endif

int WinsockInit()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    return 0;
}
void WinsockShutdown()
{
    WSACleanup();
}

#ifdef __cplusplus
}
#endif
