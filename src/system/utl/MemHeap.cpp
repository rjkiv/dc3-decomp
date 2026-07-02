#include "utl/MemHeap.h"
#include "utl/MemMgr.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "os/OSFuncs.h"
#include "utl/AllocInfo.h"
#include "utl/MakeString.h"
#include "utl/MemTracker.h"
#include "utl/TextStream.h"
#include "os/CritSec.h"
#include <cstdio>

namespace {
    int gTimeStamp;

    void
    PrintAlloc(TextStream &ts, int *iPtr, int i3, int i4, const AllocInfo *allocInfo) {
        if (i4 > 0) {
            if (i4 == 1) {
                ts << MakeString("(%p ALLOC (size %6i)", iPtr, i3);
            } else {
                ts << MakeString("(%p ALLOC (size %6i %i)", iPtr, i3, i4);
            }
            if (allocInfo) {
                for (int i = 0; i < 16; i++) {
                    if (allocInfo->mStackTrace[i] == 0U) {
                        break;
                    }
                    ts << *allocInfo;
                }
            }
            ts << MakeString(")\n");
        }
    }
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
    mIsHandleHeap = handle;
    mStrategy = strat;
    mStart = (int *)(((unsigned int)(start - 1) & ~0xF) + 0x10);
    mAllowTemp = allowTemp;
    unk24 = -1;
    mDebugLevel = debugLevel;
    mSizeWords = size - (mStart - start);
    InsertFreeBlock((FreeBlock *)mStart, mSizeWords, nullptr, nullptr, gTimeStamp++);
    if (mDebugLevel >= 1) {
        FreeBlock *end = mFreeBlockChain + mFreeBlockChain->SizeWords();
        for (FreeBlock *it = mFreeBlockChain + 1; it < end; ++it) {
            it->SetSizeWords(0xDEADDEAD);
        }
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

bool FreeBlock::AttemptMerge(FreeBlock *block, int i2) {
    if (&this[mSizeWords] == block) {
        mTimeStamp = Min(block->mTimeStamp, mTimeStamp);
        mSizeWords += block->mSizeWords;
        mNextBlock = block->mNextBlock;
        if (i2 >= 1) {
            for (int *theBlock = reinterpret_cast<int *>(block);
                 theBlock < reinterpret_cast<int *>(block + 1);
                 ++theBlock) {
                *theBlock = 0xDEADDEAD;
            }
        }
        return true;
    }
    return false;
}

int *MemHeap::Alloc(int i1, int i2, int &i3) {
    int *alloc = TryAlloc(i1, i2, i3);
    if (!alloc) {
        int lFrags, rFrags, freeBytes, i4, i5;
        FreeBlockStats(lFrags, rFrags, freeBytes, i4, i5);
        if (!MainThread()) {
            gInsideMemFunc = false;
            gMemLock->Abandon();
        }
        if (gMemTracker && !gMemTracker->GetHeapOnly()) {
            FILE *file = fopen("devkit:\\out_of_mem_alloc_info.csv", "w");
            if (file) {
                MemTracker::SpitAllocInfo(file);
                fclose(file);
            }
        }

        char buffer[2048];
        strcpy(
            buffer,
            MakeString(
                "Allocation failure, heap \"%s\", want %d bytes\n   lFrags=  %8d\n   rFrags=  %8d\n   Biggest Block=%8d\n   Free Bytes=   %8d\n",
                mName,
                i1 * 4,
                lFrags,
                rFrags,
                i5,
                freeBytes
            )
        );
        MemPrintOverview(-3, buffer + strlen(buffer));
        MILO_FAIL(buffer);
    }
    return alloc;
}
