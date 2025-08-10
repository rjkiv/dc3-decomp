#include "VorbisMem.h"
#include "utl/MemMgr.h"
#include <cstring>

void *OggMalloc(int i) { return _MemAllocTemp(i, __FILE__, 0x1C, "Ogg_Internal", 0); }

void *OggCalloc(int i1, int i2) {
    void *tmp = _MemAllocTemp(i1 * i2, __FILE__, 0x1C, "Ogg_Internal", 0);
    memset(tmp, 0, i1 * i2);
    return tmp;
}

void *OggRealloc(void *v, int i) {
    MemTemp tmp;
    return MemRealloc(v, i, __FILE__, 0x2B, "Ogg_Internal", 0);
}

void OggFree(void *v) { MemFree(v); }
