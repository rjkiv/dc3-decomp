#pragma once

namespace NUISPEECH {
    class CXboxHeap {
        struct _BLOCK_ENTRY {};

    public:
        CXboxHeap(unsigned int, unsigned int);
        ~CXboxHeap();

        void *Alloc(unsigned int, bool);
        bool Free(void *);
        void Realloc(void *, unsigned int, bool);

    private:
        _BLOCK_ENTRY *AllocatePageBlock(unsigned int);
        void InsertFreeBLockList(_BLOCK_ENTRY *);

    protected:
        CXboxHeap *mFreeHead; // 0x0
        CXboxHeap *mUsedHead; // 0x4
        CXboxHeap *mNext; // 0x8
        CXboxHeap *mPrev; // 0xc
        unsigned int mSize; // 0x10
        unsigned int mCount; // 0x14
    };
}