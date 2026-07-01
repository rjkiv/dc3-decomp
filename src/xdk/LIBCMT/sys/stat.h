#pragma once
#include "sys/types.h"
#include "xdk/LIBCMT/time.h"

#ifdef __cplusplus
extern "C" {
#endif

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

struct _stat32 { /* Size=0x24 */
    /* 0x0000 */ _dev_t st_dev;
    /* 0x0004 */ _ino_t st_ino;
    /* 0x0006 */ unsigned short st_mode;
    /* 0x0008 */ short st_nlink;
    /* 0x000a */ short st_uid;
    /* 0x000c */ short st_gid;
    /* 0x0010 */ _dev_t st_rdev;
    /* 0x0014 */ int st_size;
    /* 0x0018 */ int st_atime;
    /* 0x001c */ int st_mtime;
    /* 0x0020 */ int st_ctime;
};

struct _stat64 { /* Size=0x38 */
    /* 0x0000 */ _dev_t st_dev;
    /* 0x0004 */ _ino_t st_ino;
    /* 0x0006 */ unsigned short st_mode;
    /* 0x0008 */ short st_nlink;
    /* 0x000a */ short st_uid;
    /* 0x000c */ short st_gid;
    /* 0x0010 */ _dev_t st_rdev;
    /* 0x0018 */ long long st_size;
    /* 0x0020 */ long long st_atime;
    /* 0x0028 */ long long st_mtime;
    /* 0x0030 */ long long st_ctime;
};

int _stat64(const char *path, struct _stat64 *buffer);
int _fstat64(int fd, struct _stat64 *buffer);

#ifdef __cplusplus
}
#endif
