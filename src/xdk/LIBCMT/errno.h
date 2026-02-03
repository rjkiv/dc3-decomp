#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define errno 0

int *_errno(void);

#define EACCES 13
#define ENOSPC 28
#define ERANGE 34

#ifdef __cplusplus
}
#endif
