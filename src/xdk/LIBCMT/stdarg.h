#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef char *va_list;

// oh goodie this is an msvc intrinsic
void __va_start(va_list *, ...);

#define _SIZEOFINT(n) ((sizeof(n) + 8 - 1) & ~(8 - 1))
#define _ALIGN(ap, t) (((int)(ap) + 8 - 1) & ~(8 - 1))
/* clang-format off */
// #define va_start(ap, v) (ap = ((sizeof(v) > 8 || 0 != (sizeof(v) & (sizeof(v) - 1))) ? (va_list)(&(v)) + _SIZEOFINT(v) : (va_list)(&(v) + 1)))

#define va_start(ap, v) __va_start(&ap, &v)

#define va_arg(ap, t) (ap = (va_list)(_ALIGN(ap, t) + _SIZEOFINT(t)), (sizeof(t) > 8 || 0 != (sizeof(t) & (sizeof(t) - 1))) ? *(t *)((ap) - _SIZEOFINT(t)) : ((t *)(ap))[-1])

#define va_end(ap) (ap = (va_list)0)
/* clang-format on */
#ifdef __cplusplus
}
#endif
