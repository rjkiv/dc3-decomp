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
