#pragma once
#include "utl/MemMgr.h"
#include "utl/TextStream.h"
#include <vector>

// forward declaration
class ChunkAllocator;

#define MAX_FIXED_ALLOCS 64

class FixedSizeAlloc {
    friend class ChunkAllocator;

public:
    FixedSizeAlloc(int allocSizeWords, int nodesPerChunk);
    virtual ~FixedSizeAlloc() {}

    void *Alloc();
    void Free(void *);

    MEM_OVERLOAD(FixedSizeAlloc, 0x1C);

protected:
    virtual int *RawAlloc(int size);

    void Refill();

    int mAllocSizeWords; // 0x4
    int mNumAllocs; // 0x8
    int mMaxAllocs; // 0xc
    int mNumChunks; // 0x10
    int *mFreeList; // 0x14
    int mNodesPerChunk; // 0x18
};

class ChunkAllocator {
public:
    ChunkAllocator();
    void *Alloc(int);
    void Free(void *, int);
    void Print(TextStream &);

    MEM_OVERLOAD(ChunkAllocator, 0x38);

private:
    FixedSizeAlloc *mAllocs[MAX_FIXED_ALLOCS]; // 0x0
};

class ReclaimableAlloc : public FixedSizeAlloc {
public:
    ReclaimableAlloc(int, const char *name);

    void *CustAlloc(int bytes);
    void CustFree(void *mem);

protected:
    virtual int *RawAlloc(int size);

    void DeallocAll();

    const char *mName; // 0x1c
    std::vector<void *> mChunks; // 0x20
};

void PoolAllocInit(class DataArray *cfg);
void *PoolAlloc(int classSize, int reqSize, const char *file, int line, const char *name);
void PoolFree(int, void *mem, const char *file, int line, const char *name);
void PoolReport(TextStream &);

#define POOL_OVERLOAD(class_name, line_num)                                              \
    static void *operator new(size_t s) {                                          \
        return PoolAlloc(s, s, __FILE__, line_num, #class_name);                         \
    }                                                                                    \
    static void *operator new(size_t s, void *place) { return place; }             \
    static void operator delete(void *v) {                                               \
        PoolFree(sizeof(class_name), v, __FILE__, line_num, #class_name);                \
    }
