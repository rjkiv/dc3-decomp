#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_ASSERT_H__ 202311L
#ifdef NDEBUG
#define assert(...) ((void)0)
#else
#define assert(...) /* implementation-defined */
#endif
#else
#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
#define assert(condition) /* implementation-defined */
#endif
#endif

#ifdef __cplusplus
}
#endif
