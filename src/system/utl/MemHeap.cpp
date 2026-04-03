#include "utl/MemHeap.h"
#include "MemHeap.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/TextStream.h"

namespace {
    int gTimeStamp;
}

int MemHeap::GetSizeWords(int size) {
    unsigned int words = ((size + 3) >> 2) + 1;
    if (words >= 3)
        return words;
    return 3;
}

void MemHeap::FreeBlockStats(int &lFrags, int &rFrags, int &freeBytes, int &i4, int &i5) {
    int i = 0;
    int ivar5 = 0;
    int ivar3 = 0;
    int ivar6 = -1;
    for (FreeBlock *it = mFreeBlockChain; it != nullptr; it = it->NextBlock(), i++) {
        int size = it->SizeWords() * 4;
        if (ivar5 < size) {
            ivar5 = size;
            ivar6 = i;
        }
        ivar3 += size;
    }
    freeBytes = ivar3;
    i5 = ivar5;
    lFrags = ivar6;
    rFrags = (i - ivar6) - 1;
    unk24 = Min<unsigned int>(ivar3, unk24);
    i4 = unk24;
}

void MemHeap::Print(TextStream &ts, bool b2) {
    ts << MakeString(";---------------------------------------\n");
    int sizeBytes = mSizeWords * 4;
    ts << MakeString(
        "; HEAP: %i (%s), starts %p, %d bytes\n", mNum, mName, mStart, sizeBytes
    );
    int lFrags, rFrags, i10b0, i10b4;
    FreeBlockStats(lFrags, rFrags, sizeBytes, i10b0, i10b4);
    ts << MakeString("\n");
    ts << MakeString(
        ";   lFrags =  %8d\n;   rFrags =  %8d\n;   Total Free Bytes=  %8d\n",
        lFrags,
        rFrags,
        sizeBytes
    );
    ts << MakeString("\n");
    for (int i = sizeBytes; i < sizeBytes + mSizeWords * 4; i++) {
    }
}

void MemHeap::InsertFreeBlock(
    FreeBlock *iBlock, int size, FreeBlock *iPrevBlock, FreeBlock *iNextBlock, int time
) {
    MILO_ASSERT((iBlock != iPrevBlock) && (iBlock != iNextBlock), 0x68);
    iBlock->SetSizeWords(size);
    iBlock->SetNextBlock(iNextBlock);
    iBlock->SetTimestamp(time);
    if (iPrevBlock) {
        iPrevBlock->SetNextBlock(iBlock);
    } else {
        mFreeBlockChain = iBlock;
    }
}

void MemHeap::Init(
    const char *name,
    int num,
    int *start,
    int size,
    bool handle,
    Strategy strat,
    int debugLevel,
    bool allowTemp
) {
    MILO_ASSERT_FMT(start, "Could not allocate %d bytes for heap %s\n", size * 4, name);
    mStart = start;
    mName = name;
    mNum = num;
    int *i7 = (start - 1) + 0x10;
    mIsHandleHeap = handle;
    mStrategy = strat;
    mStart = i7;
    mAllowTemp = allowTemp;
    unk24 = -1;
    mDebugLevel = debugLevel;
    mSizeWords = size - (i7 - start >> 2);
    gTimeStamp++;
    InsertFreeBlock((FreeBlock *)mStart, mSizeWords, nullptr, nullptr, gTimeStamp);
    if (mDebugLevel > 0) {
    }
}

void MemHeap::FirstFit(int size, int align, FreeBlockInfo &blockinfo) {
    FreeBlock *prev = nullptr;
    for (auto block = mFreeBlockChain; block != nullptr; block = block->NextBlock()) {
        int start = ((int)block >> 2) + 1;
        int alignment = start + (1 << align) - 1 >> (1 << align);
        int pad = alignment - start;
        if ((int)block->SizeWords() >= pad + size) {
            blockinfo.mSizeWords = block->SizeWords();
            blockinfo.mPadWords = pad;
            blockinfo.mBlock = block;
            blockinfo.mPrevBlock = prev;
            return;
        }
    }
}
