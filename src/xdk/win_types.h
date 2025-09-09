#pragma once

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONST const

typedef int BOOL;
typedef u8 BYTE;
typedef u32 DWORD;
typedef u64 DWORDLONG;
typedef u32 DWORD32;
typedef u64 DWORD64;

typedef char CCHAR;
typedef char CHAR;
typedef s32 INT;
typedef s32 INT_PTR;
typedef s8 INT8;
typedef s16 INT16;
typedef s32 INT32;
typedef s64 INT64;
typedef s32 LONG;
typedef s64 LONGLONG;
typedef s32 LONG_PTR;
typedef s32 LONG32;
typedef s64 LONG64;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef float FLOAT;
typedef uint UINT;

typedef void *HANDLE;
typedef HANDLE HDC;
typedef HANDLE HTASK;
typedef HANDLE HINSTANCE;
typedef s32 HFILE;
typedef HINSTANCE HMODULE;
typedef LONG HRESULT;

typedef void *PVOID;
typedef const void *LPCVOID;
typedef BOOL *LPBOOL;
typedef BYTE *LPBYTE;
typedef char *LPSTR;
typedef wchar_t *LPWSTR;
typedef const char *LPCSTR;
typedef const wchar_t *LPCWSTR;
typedef DWORD *LPDWORD;
typedef HANDLE *LPHANDLE;

typedef DWORD FOURCC;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                   \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16)         \
     | ((DWORD)(BYTE)(ch3) << 24))

#ifdef __cplusplus
}
#endif
