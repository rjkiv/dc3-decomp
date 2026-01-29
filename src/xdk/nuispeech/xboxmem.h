#pragma once
#include "xdk/xapilibi/rtlheap.h"
#include "xdk/xapilibi/xbox.h"

namespace NUISPEECH {
    unsigned long GetXAllocAttributes(int);
    void *MemAlloc(void *heap, unsigned long size, unsigned long attrs);
    void MemFree(void *, void *, unsigned long);
    unsigned long MemSize(void *, void *, unsigned long);
    void *MemReAlloc(void *, int, void *, unsigned long);
}