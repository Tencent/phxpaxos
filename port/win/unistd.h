#pragma once

#include <io.h>
#include <direct.h>
#include <conio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define F_OK 0
#define S_IRWXU 00700
#define S_IRWXG 00070
#define S_IROTH 00004
#define S_IXOTH 00001

typedef long ssize_t;

#define mkdir(x, y) _mkdir(x)


int pipe(int pipefd[2]);
int fsync(int fd);
int fdatasync(int fd);
int truncate(const char *path, long long length);
unsigned int sleep(unsigned int seconds);

#ifdef __cplusplus
};
#endif
