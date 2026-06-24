#include "utl/PoolAlloc.h"
#include "MemMgr.h"
#include "math/Utl.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "obj/Data.h"
#include "utl/TextStream.h"
#include "utl/Std.h"
#include <cstdio>

static int gHunkSize = 0xC800;
static int gSmallHunkSize = 0xC800;

static int gTotalChunksSize = 0;
static bool gPoolAllocInitted = 0;
ChunkAllocator *gChunkAlloc = nullptr;

void PoolAllocInit(DataArray *a) {
    a->FindData("big_hunk", gHunkSize);
    gPoolAllocInitted = true;
}

void *
PoolAlloc(int classSize, int reqSize, const char *file, int line, const char *name) {
    MILO_ASSERT_FMT(classSize >= 0, "PoolAlloc class size is < 0: %d", classSize);
    CritSecTracker tracker(gMemLock);
    if (!gChunkAlloc) {
        gChunkAlloc = new ChunkAllocator();
    }
    MILO_ASSERT(reqSize == classSize, 0x15F);
    void *alloced = gChunkAlloc->Alloc(classSize);
    MemTrackAlloc(classSize, classSize, name, alloced, true, 0, file, line);
    return alloced;
}

void PoolFree(int idx, void *mem, const char *file, int line, const char *name) {
    CritSecTracker tracker(gMemLock);
    MemTrackFree(mem);
    MILO_ASSERT(gChunkAlloc, 0x16F);
    gChunkAlloc->Free(mem, idx);
}

void PoolReport(TextStream &ts) {
    CritSecTracker tracker(gMemLock);
    MILO_ASSERT(gChunkAlloc, 0x179);
    gChunkAlloc->Print(ts);
}

#pragma region FixedSizeAlloc

FixedSizeAlloc::FixedSizeAlloc(int allocSizeWords, int nodesPerChunk)
    : mAllocSizeWords(allocSizeWords), mNumAllocs(0), mMaxAllocs(0), mNumChunks(0),
      mFreeList(nullptr), mNodesPerChunk(nodesPerChunk) {
    MILO_ASSERT(mAllocSizeWords != 0, 0x9D);
}

void *FixedSizeAlloc::Alloc() {
    if (!mFreeList) {
        Refill();
    }
    int *ret = mFreeList;
    int numAllocs = mNumAllocs + 1;
    int *next = (int *)*ret;
    mNumAllocs = numAllocs;
    mFreeList = next;
    if (numAllocs > mMaxAllocs) {
        mMaxAllocs = numAllocs;
    }
    return ret;
}

void FixedSizeAlloc::Free(void *v) {
    *(int **)v = mFreeList;
    mFreeList = (int *)v;
    MILO_ASSERT_FMT(mNumAllocs > 0, "mNumAllocs is %d", mNumAllocs);
    mNumAllocs--;
}

int *FixedSizeAlloc::RawAlloc(int size) {
    static int *gPoolEnd = nullptr;
    static int *gPoolStart = nullptr;
    gTotalChunksSize += size;
    if (gPoolStart + (size >> 2) > gPoolEnd) {
        if (MemNumHeaps() > 0) {
            if (gHunkSize == gSmallHunkSize) {
                printf("PoolAlloc warning: allocating small pool chunk\n");
            }
            MemPushHeap(0);
        }
        gPoolStart = (int *)_MemAllocTemp(gHunkSize, __FILE__, 0x71, "PoolChunk", 0);
        if (MemNumHeaps() > 0) {
            MemPopHeap();
        }
        gPoolEnd = gPoolStart + (gHunkSize >> 2);
        gHunkSize = gSmallHunkSize;
        gPoolStart += 0x10;
    }
    int *ret = gPoolStart;
    gPoolStart += (size >> 2);
    return ret;
}

void FixedSizeAlloc::Refill() {
    MILO_ASSERT(mFreeList == 0, 0xCA);
    int allocSize = mAllocSizeWords * mNodesPerChunk;
    mFreeList = RawAlloc(allocSize * 4);
    mNumChunks++;

    int *it = mFreeList;
    int *itEnd = mFreeList + (allocSize - mAllocSizeWords);
    for (; it < itEnd; it += mAllocSizeWords) {
        *it += mAllocSizeWords;
    }
    *it = 0;
}

#pragma endregion
#pragma region ChunkAllocator

ChunkAllocator::ChunkAllocator() {
    for (int i = 0; i < MAX_FIXED_ALLOCS; i++) {
        mAllocs[i] = new FixedSizeAlloc((i + 1) * 4, 20);
    }
}

void *ChunkAllocator::Alloc(int idx) {
    int fixedSizeIndex = (idx - 1) >> 4;
    MILO_ASSERT(fixedSizeIndex < MAX_FIXED_ALLOCS, 0x116);
    return mAllocs[fixedSizeIndex]->Alloc();
}

void ChunkAllocator::Free(void *v, int idx) {
    int fixedSizeIndex = (idx - 1) >> 4;
    MILO_ASSERT(fixedSizeIndex < MAX_FIXED_ALLOCS, 0x122);
    MILO_ASSERT(mAllocs[fixedSizeIndex], 0x123);
    mAllocs[fixedSizeIndex]->Free(v);
}

void ChunkAllocator::Print(TextStream &ts) {
    ts << MakeString("\n*** POOL REPORT (Total Capacity: %d)***\n", gTotalChunksSize);
    ts << MakeString("   NodeSize   NumAllocs  MaxAllocs  Capacity  Wasted\n");
    int wasted = 0;
    for (int i = 0; i < 64; i++) {
        if (mAllocs[i]) {
            FixedSizeAlloc *cur = mAllocs[i];
            int numAllocs = cur->mNumAllocs;
            int capacity = cur->mNodesPerChunk * cur->mNumChunks;
            int maxAllocs = cur->mMaxAllocs;
            int nodeSize = cur->mAllocSizeWords * 4;
            int curWasted = (capacity - cur->mNumAllocs) * cur->mAllocSizeWords * 4;
            ts << MakeString(
                "   %8d  %8d  %8d  %8d  %8d\n",
                nodeSize,
                numAllocs,
                maxAllocs,
                capacity,
                curWasted
            );
            wasted += curWasted;
        }
    }
    ts << MakeString("                             Total Waste = %8d\n", wasted);
}

#pragma endregion
#pragma region ReclaimableAlloc

ReclaimableAlloc::ReclaimableAlloc(int x, const char *name)
    : FixedSizeAlloc(((x + 15) >> 2) & ~3, 0x2800 / x), mName(name) {}

int *ReclaimableAlloc::RawAlloc(int num) {
    void *alloced = MemAlloc(num, __FILE__, 0x196, mName);
    mChunks.push_back(alloced);
    return (int *)alloced;
}

void *ReclaimableAlloc::CustAlloc(int bytes) {
    MILO_ASSERT(bytes <= mAllocSizeWords * 4, 0x188);
    return Alloc();
}

void ReclaimableAlloc::CustFree(void *mem) {
    Free(mem);
    if (mNumAllocs == 0) {
        DeallocAll();
    }
}

void ReclaimableAlloc::DeallocAll() {
    MILO_ASSERT(mNumAllocs == 0, 0x19D);
    FOREACH (it, mChunks) {
        MemFree(*it);
    }
    mChunks.clear();
    mNumChunks = 0;
    mFreeList = nullptr;
}
