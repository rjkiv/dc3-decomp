#pragma once

// size 0x14
class HeapStats {
public:
    HeapStats()
        : mTotalNumAllocs(0), mTotalActSize(0), mTotalReqSize(0), mMaxNumAllocs(0),
          mMaxActSize(0) {}
    void Alloc(int, int);
    void Free(int, int);

    int mTotalNumAllocs; // 0x0
    int mTotalActSize; // 0x4
    int mTotalReqSize; // 0x8
    int mMaxNumAllocs; // 0xc
    int mMaxActSize; // 0x10
};

// size 0x18
class BlockStat {
public:
    const char *mName; // 0x0
    int mSizeReq; // 0x4
    int mSizeAct; // 0x8
    int mMaxSize; // 0xc
    int mNumAllocs; // 0x10
    unsigned char mHeap; // 0x14
};

// size 0x600c
class BlockStatTable {
private:
    BlockStat mStats[0x400]; // 0x0
    int mMaxStats; // 0x6000
    int mNumStats; // 0x6004
    bool mSizeMatters; // 0x6008
public:
    BlockStatTable(bool sizeMatters = false);
    void Clear();
    void SortBySize();
    void SortByName();
    void Update(const char *type, unsigned char heap, int reqSize, int actSize);
    BlockStat &GetBlockStat(int iStat);
    int GetNumStats() const { return mNumStats; }
};
