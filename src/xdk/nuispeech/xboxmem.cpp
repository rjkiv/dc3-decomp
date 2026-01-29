#include "xboxmem.h"

unsigned long NUISPEECH::GetXAllocAttributes(int i) {
    return (unsigned int)(i != 0) << 0x1e | 0x249b0000;
}

void *NUISPEECH::MemAlloc(void *heap, unsigned long size, unsigned long attrs) {
    if (heap == nullptr) {
        return XMemAlloc(size, attrs);
    }
    unsigned long flags = (attrs >> 0x1b) & 8;
    return RtlAllocateHeap(heap, flags, size);
}

void NUISPEECH::MemFree(void *v1, void *v2, unsigned long ul) {
    if (v1 == nullptr) {
        XMemFree(v2, ul);
        return;
    }
    RtlFreeHeap(v1, 0, v2);
}

unsigned long NUISPEECH::MemSize(void *v1, void *v2, unsigned long ul) {
    if (v1 == nullptr) {
        return XMemSize(v2, ul);
    }
    return RtlSizeHeap(v1, 0, v2);
}