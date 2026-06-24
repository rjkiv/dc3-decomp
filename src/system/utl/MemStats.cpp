#include "utl/MemStats.h"
#include "math/Utl.h"
#include "os/Debug.h"

int SizeLess(const void *v1, const void *v2) {
    const BlockStat *b1 = (const BlockStat *)v1;
    const BlockStat *b2 = (const BlockStat *)v2;
    if (b1->mSizeAct < b2->mSizeAct) {
        return 1;
    } else if (b2->mSizeAct < b1->mSizeAct) {
        return -1;
    } else
        return 0;
}

int NameLess(const void *v1, const void *v2) {
    const BlockStat *b1 = (const BlockStat *)v1;
    const BlockStat *b2 = (const BlockStat *)v2;
    return strcmp(b1->mName, b2->mName);
}

BlockStatTable::BlockStatTable(bool sizeMatters)
    : mMaxStats(0x400), mNumStats(0), mSizeMatters(sizeMatters) {}

void BlockStatTable::Clear() { mNumStats = 0; }

void BlockStatTable::SortBySize() {
    qsort(mStats, mNumStats, sizeof(BlockStat), SizeLess);
}

void BlockStatTable::SortByName() {
    qsort(mStats, mNumStats, sizeof(BlockStat), NameLess);
}

BlockStat &BlockStatTable::GetBlockStat(int iStat) {
    MILO_ASSERT((0) <= (iStat) && (iStat) < (mNumStats), 0x37);
    return mStats[iStat];
}

void BlockStatTable::Update(
    const char *type, unsigned char heap, int reqSize, int actSize
) {
    int idx = 0;
    for (; idx < mNumStats; idx++) {
        if (mStats[idx].mHeap == heap
            && (!mSizeMatters || mStats[idx].mSizeReq == reqSize)) {
            if (streq(mStats[idx].mName, type)) {
                if (!mSizeMatters) {
                    mStats[idx].mSizeReq += reqSize;
                }
                mStats[idx].mSizeAct += actSize;
                mStats[idx].mMaxSize =
                    reqSize >= mStats[idx].mMaxSize ? reqSize : mStats[idx].mMaxSize;
                mStats[idx].mNumAllocs++;
                return;
            }
        }
    }
    if (idx == mNumStats && mNumStats < mMaxStats) {
        mStats[mNumStats].mName = type;
        mStats[mNumStats].mHeap = heap;
        mStats[mNumStats].mSizeReq = reqSize;
        mStats[mNumStats].mMaxSize = reqSize;
        mStats[mNumStats].mSizeAct = actSize;
        mStats[mNumStats].mNumAllocs = 1;
        mNumStats++;
    } else {
        MILO_FAIL("Stack overflow in BlockStatTable!");
    }
}

void HeapStats::Alloc(int act, int req) {
    mTotalNumAllocs++;
    mTotalActSize += act;
    mTotalReqSize += req;
    mMaxNumAllocs = Max(mTotalNumAllocs, mMaxNumAllocs);
    mMaxActSize = Max(mTotalActSize, mMaxActSize);
}

void HeapStats::Free(int act, int req) {
    mTotalNumAllocs--;
    mTotalActSize -= act;
    mTotalReqSize -= req;
}
