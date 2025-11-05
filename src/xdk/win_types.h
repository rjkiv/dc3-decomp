#pragma once
#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONST const
#define CALLBACK __stdcall
#define WINAPI __stdcall

typedef void VOID, *PVOID, *LPVOID;
typedef CONST VOID *LPCVOID;

typedef u8 BYTE, *PBYTE, *LPBYTE;
typedef int BOOL, *PBOOL, *LPBOOL;
typedef BYTE BOOLEAN, *PBOOLEAN;

typedef char CCHAR, CHAR, *PCHAR, *PSTR, *LPSTR;
typedef CONST CHAR *PCSTR, *LPCSTR;
typedef s8 INT8, *PINT8;
typedef u8 UCHAR, *PUCHAR;
typedef u8 UINT8, *PUINT8;

typedef s16 HALF_PTR, *PHALF_PTR;
typedef s16 SHORT, *PSHORT;
typedef signed short INT16;
typedef INT16 *PINT16;

typedef u16 UHALF_PTR, *PUHALF_PTR;
typedef u16 USHORT, *PUSHORT;
typedef u16 UINT16, *PUINT16;
typedef u16 WORD, *PWORD, *LPWORD;

typedef wchar_t WCHAR, *PWCHAR, *PWSTR, *LPWSTR;
typedef CONST WCHAR *PCWSTR, *LPCWSTR;

typedef float FLOAT, *PFLOAT;

typedef int INT, *PINT, *LPINT;
typedef int INT_PTR, *PINT_PTR;
typedef signed int INT32, *PINT32;

typedef uint UINT, *PUINT;
typedef uint UINT_PTR, *PUINT_PTR;
typedef uint UINT32, *PUINT32;
typedef uint ULONG32, *PULONG32;

typedef s32 LONG, *PLONG, *LPLONG;
typedef s32 LONG_PTR, *PLONG_PTR;
typedef signed int LONG32, *PLONG32;

typedef u32 ULONG, *PULONG;
typedef u32 ULONG_PTR, *PULONG_PTR;

typedef u32 DWORD, *PDWORD, *LPDWORD;
typedef uint DWORD32, *PDWORD32;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;

typedef s64 LONGLONG, *PLONGLONG;
typedef signed long long INT64, *PINT64;
typedef s64 LONG64, *PLONG64;
typedef s64 LARGE_INTEGER, *PLARGE_INTEGER;

typedef u64 QWORD;
typedef u64 ULONGLONG, *PULONGLONG;
typedef u64 UINT64, *PUINT64;
typedef u64 ULONG64, *PULONG64;
typedef u64 DWORDLONG, *PDWORDLONG;
typedef u64 DWORD64, *PDWORD64;

typedef union _ULARGE_INTEGER { /* Size=0x8 */
    struct {
        /* 0x0000 */ DWORD HighPart;
        /* 0x0004 */ DWORD LowPart;
    };
    struct {
        /* 0x0000 */ DWORD HighPart;
        /* 0x0004 */ DWORD LowPart;
    } u;
    /* 0x0000 */ ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef UINT_PTR WPARAM;

typedef PVOID HANDLE, *LPHANDLE;
typedef HANDLE HDC;
typedef HANDLE HTASK;
typedef HANDLE HINSTANCE;
typedef int HFILE;
typedef HINSTANCE HMODULE;
typedef LONG HRESULT;

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR) - 1)

typedef DWORD FOURCC;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                   \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16)         \
     | ((DWORD)(BYTE)(ch3) << 24))

#ifdef __cplusplus
}
#endif
