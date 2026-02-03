#pragma once

#include "sys/types.h"
#include "xdk/LIBCMT/time.h"
struct _stati64 {
    _dev_t st_dev;
    _ino_t st_ino;
    unsigned short st_mode;
    short st_nlink;
    short st_uid;
    short st_gid;
    _dev_t st_rdev;
    long st_size;
    __time64_t st_atime;
    __time64_t st_mtime;
    __time64_t st_ctime;
};

struct __stat64 {
    _dev_t st_dev;
    _ino_t st_ino;
    unsigned short st_mode;
    short st_nlink;
    short st_uid;
    short st_gid;
    _dev_t st_rdev;
    __int64 st_size;
    __time64_t st_atime;
    __time64_t st_mtime;
    __time64_t st_ctime;
};

#ifdef __cplusplus
extern "C" {
#endif
int _stat64(const char *path, struct __stat64 *buffer);
int _fstat64(int fd, struct __stat64 *buffer);
#ifdef __cplusplus
}
#endif
