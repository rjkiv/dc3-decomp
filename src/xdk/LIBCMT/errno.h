#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern int *_errno(void);

#define errno (*_errno())

#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define ENOSPC 28
#define ERANGE 34

#ifdef __cplusplus
}
#endif
