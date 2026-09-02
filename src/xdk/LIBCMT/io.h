#pragma once

// these are prolly just like
// #define fwrite(buf, size, n, fi) _write(fi->fd, buf, size*n)
// but idk for sure. ms *does* publically list the POSIX vers
int _open(const char *filename, int oflag, int pmode);

#ifdef __cplusplus
extern "C" {
#endif

int _write(int fd, const void *buf, int len);
int _close(int fd);
long _lseek(int fd, long offset, int origin);
long long _lseeki64(int fd, long long offset, int origin);

#ifdef __cplusplus
}
#endif
