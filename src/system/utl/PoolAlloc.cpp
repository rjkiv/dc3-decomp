#include "utl/PoolAlloc.h"
#include "MemMgr.h"
#include "math/Utl.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "obj/Data.h"
#include "utl/TextStream.h"

int gBigHunk = 0xC800;
bool gPoolAllocInitted;
int gPoolCapacity;
ChunkAllocator *gChunkAlloc;

void PoolAllocInit(DataArray *a) {
    a->FindData("big_hunk", gBigHunk, true);
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

FixedSizeAlloc::FixedSizeAlloc(int x, int y)
    : mAllocSizeWords(x), mNumAllocs(0), mMaxAllocs(0), mNumChunks(0), mFreeList(nullptr),
      mNodesPerChunk(y) {
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

void FixedSizeAlloc::Refill() {
    MILO_ASSERT(mFreeList == 0, 0xCA);
    int allocSize = mAllocSizeWords * mNodesPerChunk;
    mFreeList = RawAlloc(allocSize * 4);
    mNumChunks++;

    int *it = mFreeList;
    for (; it < mFreeList + (allocSize - mAllocSizeWords); ++it) {
        *it += mAllocSizeWords;
    }
    *it = 0;
}

ChunkAllocator::ChunkAllocator() {
    for (int i = 0; i < 64; i++) {
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
    ts << MakeString("\n*** POOL REPORT (Total Capacity: %d)***\n", gPoolCapacity);
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
    for (std::vector<void *>::iterator it = mChunks.begin(); it != mChunks.end(); ++it) {
        MemFree(*it);
    }
    mChunks.clear();
    mNumChunks = 0;
    mFreeList = nullptr;
}
