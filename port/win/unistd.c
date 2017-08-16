#include "unistd.h"
#include <windows.h>
#include <fcntl.h>


int pipe(int pipefd[2])
{
    return _pipe(pipefd, 256, O_BINARY);
}

int fsync(int fd)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE)
        return -1;
    if (FlushFileBuffers(h))
        return 0;
    else
        return -1;
}
int fdatasync(int fd)
{
    return fsync(fd);
}

int truncate(const char *path, long long length)
{
    HANDLE h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        errno = GetLastError();
        return -1;
    }

    LARGE_INTEGER len, newPointer;
    len.QuadPart = length;
    if (!SetFilePointerEx(h, len, &newPointer, FILE_BEGIN))
    {
        errno = GetLastError();
        CloseHandle(h);
        return -1;
    }
    SetEndOfFile(h);
    CloseHandle(h);

    return 0;
}

unsigned int sleep(unsigned int seconds)
{
    Sleep(1000 * seconds);
    return 0;
}
