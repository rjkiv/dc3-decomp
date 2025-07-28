#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <xdk/win_types.h>

typedef HANDLE HMMIO;
typedef LPSTR HPSTR;
typedef LRESULT MMRESULT;

typedef LRESULT (*LPMMIOPROC)(PVOID lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2);

typedef struct _MMIOINFO {
    DWORD dwFlags; // 0x0
    FOURCC fccIOProc; // 0x4
    LPMMIOPROC pIOProc; // 0x8
    UINT wErrorRet; // 0xc
    HTASK hTask; // 0x10
    LONG cchBuffer; // 0x14
    HPSTR pchBuffer; // 0x18
    HPSTR pchNext; // 0x1c
    HPSTR pchEndRead; // 0x20
    HPSTR pchEndWrite; // 0x24
    LONG lBufOffset; // 0x28
    LONG lDiskOffset; // 0x2C
    DWORD adwInfo[4]; // 0x30
    DWORD dwReserved1; // 0x40
    DWORD dwReserved2; // 0x44
    HMMIO hmmio; // 0x48
} MMIOINFO, *LPMMIOINFO;
typedef const LPMMIOINFO LPCMMIOINFO;

MMRESULT mmioGetInfo(HMMIO hmmio, LPMMIOINFO pmmioinfo, UINT fuInfo);
MMRESULT mmioSetInfo(HMMIO hmmio, LPCMMIOINFO pmmioinfo, UINT fuInfo);
MMRESULT mmioFlush(HMMIO hmmio, UINT fuFlush);
LONG mmioWrite(HMMIO hmmio, const char *pch, LONG cch);
MMRESULT mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT fuBuffer);
MMRESULT mmioClose(HMMIO hmmio, UINT fuClose);
LONG mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin);
LONG mmioRead(HMMIO hmmio, HPSTR pch, LONG cch);
HMMIO mmioOpenW(LPWSTR pszFileName, LPMMIOINFO pmmioinfo, DWORD fdwOpen);
MMRESULT mmioAdvance(HMMIO hmmio, LPMMIOINFO pmmioinfo, UINT fuAdvance);

FOURCC mmioStringToFOURCCW(LPCSTR sz, UINT uFlags);

#ifdef __cplusplus
}
#endif
