#pragma once

class FreeBlock {
public:
    FreeBlock *NextBlock() { return mNextBlock; }
    unsigned int SizeWords() { return mSizeWords; }
    unsigned int TimeStamp() const { return mTimeStamp; }
    void SetNextBlock(FreeBlock *block) { mNextBlock = block; }
    void SetTimestamp(unsigned int time) { mTimeStamp = time; }
    void SetSizeWords(unsigned int size) { mSizeWords = size; }

    //   public: int32_t* StartAddr();
    //   public: int32_t* EndAddr();
    //   public: int32_t* SpaceAddr();
    //   public: uint64_t SpaceWords();
    //   public: bool AttemptMerge(FreeBlock*, int32_t);
    //   public: uint64_t CalcPadWords(uint64_t, uint64_t);
    //   public: int64_t BackAllocPadWords(uint64_t, int32_t, uint64_t);
    //   public: void Set(uint64_t, FreeBlock*, uint32_t);
private:
    unsigned int mSizeWords;
    unsigned int mTimeStamp;
    FreeBlock *mNextBlock;
};

class MemHeap {
public:
    enum Strategy {
        kFirstFit = 0,
        kBestFit = 1,
        kLRUFit = 2,
        kLastFit = 3,
    };
    struct FreeBlockInfo {
        FreeBlock *mBlock;
        FreeBlock *mPrevBlock;
        int mSizeWords;
        int mPadWords;
    };

    int Free(int *);
    int *Truncate(int *, int, int &);
    void Print(class TextStream &, bool);
    void Init(const char *, int, int *, int, bool, Strategy, int, bool);
    int AllocSize(int *);
    void FreeBlockStats(int &, int &, int &, int &, int &);
    void FirstFit(int, int, FreeBlockInfo &);
    void BestFit(int, int, FreeBlockInfo &);
    void LRUFit(int, int, FreeBlockInfo &);
    void LastFit(int, int, FreeBlockInfo &);

    const char *Name() const { return mName; }
    int SizeWords() const { return mSizeWords; }
    int *Start() const { return mStart; }
    int *End() const { return mStart + mSizeWords; }

    static int GetSizeWords(int);

private:
    void InsertFreeBlock(FreeBlock *, int, FreeBlock *, FreeBlock *, int);

    FreeBlock *mFreeBlockChain; // 0x0
    int *mStart; // 0x4
    const char *mName; // 0x8
    int mSizeWords; // 0xc
    int mNum; // 0x10
    bool mIsHandleHeap; // 0x14
    int mDebugLevel; // 0x18
    Strategy mStrategy; // 0x1c
    bool mAllowTemp; // 0x20
    int unk24; // 0x24
};

class MemHeapStack {
public:
    int mStack[16]; // 0x0
    int mSize; // 0x40
    int mTempRefs; // 0x44

    static int sDefaultHeap;
};
