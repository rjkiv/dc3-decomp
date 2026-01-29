#include "xboxheap.h"

NUISPEECH::CXboxHeap::CXboxHeap(unsigned int initSize, unsigned int size) {
    mCount = 0;
    mSize = size;
    mFreeHead = this;
    mUsedHead = this;
    mNext = nullptr;
    mPrev = nullptr;
    AllocatePageBlock(initSize);
}