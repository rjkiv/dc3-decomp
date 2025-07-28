#include "mmio.h"
#include <stdlib.h>

void FreeHandle(HANDLE);

MMRESULT mmioGetInfo(HMMIO hmmio, LPMMIOINFO pmmioinfo, UINT fuInfo) {
    if (hmmio == nullptr) {
        return 5;
    }
    if (pmmioinfo == nullptr) {
        return 11;
    }
    memcpy(pmmioinfo, hmmio, 0x48); // should be sizeof(MMIOINFO) but fsr mine is 4 bytes
                                    // larger
    return 0;
}

MMRESULT mmioSetInfo(HMMIO hmmio, LPCMMIOINFO pmmioinfo, UINT fuInfo) {
    if (hmmio == nullptr) {
        return 5;
    }
    if (pmmioinfo == nullptr) {
        return 11;
    }
    memcpy(hmmio, pmmioinfo, 0x48); // should be sizeof(MMIOINFO) but fsr mine is 4 bytes
                                    // larger
    LPMMIOINFO new_info = (LPMMIOINFO)hmmio;
    if (new_info->pchEndRead < new_info->pchNext) {
        new_info->pchEndRead = new_info->pchNext;
    }
    return 0;
}

FOURCC mmioStringToFOURCCW(LPCSTR sz, UINT uFlags) { return 0; }

MMRESULT mmioFlush(HMMIO hmmio, UINT fuFlush) { return 0; }

LONG mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin) { return 0; }

MMRESULT mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT fuBuffer) {
    return 0;
}

HMMIO mmioOpenW(LPWSTR pszFileName, LPMMIOINFO pmmioinfo, DWORD fdwOpen) {
    return nullptr;
}

MMRESULT mmioClose(HMMIO hmmio, UINT fuClose) {
    if (hmmio == nullptr) {
        return 5;
    }
    uint flush_ret = mmioFlush(hmmio, 0);
    if (flush_ret != 0)
        return flush_ret;
    LPMMIOINFO info = (LPMMIOINFO)hmmio;
    uint proc_ret = info->pIOProc(info, 4, fuClose, 0);
    if (proc_ret != 0)
        return proc_ret;

    mmioSetBuffer(hmmio, 0, 0, 0);
    FreeHandle(hmmio);

    return 0;
}

MMRESULT mmioAdvance(HMMIO hmmio, LPMMIOINFO pmmioinfo, UINT fuAdvance) { return 0; }

LONG mmioRead(HMMIO hmmio, HPSTR pch, LONG cch) { return 0; }

LONG mmioWrite(HMMIO hmmio, const char *pch, LONG cch) { return 0; }
