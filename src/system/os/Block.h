#pragma once
#include "os/AsyncTask.h"
#include "os/Timer.h"
#include "types.h"
#include "utl/PoolAlloc.h"

class Block {
public:
    Block();
    void UpdateTimestamp();

    bool Matches(int arknum, int blocknum) const {
        return mArkfileNum == arknum && mBlockNum == blocknum;
    }

    bool Matches(const class BlockRequest &) const;

    MEM_OVERLOAD(Block, 0x16);
    static int CurrentTimestamp() { return sCurrTimestamp; }

    char *mBuffer; // 0x0
    int mArkfileNum; // 0x4
    int mBlockNum; // 0x8
    int mTimestamp; // 0xc
    bool mWritten; // 0x10
    const char *mDebugName; // 0x14

private:
    static int sCurrTimestamp;
};

class BlockRequest {
public:
    BlockRequest(const AsyncTask &);

    bool Matches(int arknum, int blocknum) const {
        return arknum == mArkfileNum && blocknum == mBlockNum;
    }

    // should be "Exceeds"
    bool LessThan(int arknum, int blocknum) const {
        return (arknum < mArkfileNum) || (mArkfileNum == arknum && mBlockNum > blocknum);
    }

    // public: bool Matches(const BlockRequest&) const;
    // public: bool Exceeds(int32_t, int32_t) const;

    int mArkfileNum; // 0x0
    int mBlockNum; // 0x4
    const char *mDebugName; // 0x8
    std::list<AsyncTask> mTasks; // 0xc
};

class BlockMgr {
public:
    char *GetBlockData(int arkfileNum, int iBlockNum);
    void KillBlockRequests(ArkFile *iArk);
    void Poll();
    void GetAssociatedBlocks(
        u64 byteOffset, int numBytes, int &oStartBlock, int &oNumBlocks, int &oBlockSize
    );
    void AddTask(const AsyncTask &);
    bool SpinUp();
    void Init();
    void MarkDiscRead();

private:
    void WriteBlock();
    void ReadBlock();
    Block *FindBlock(int arkfileNum, int iBlockNum);
    Block *FindLRUBlock(bool write); // Least Recently Updated
    Block *FindMRUBlock(); // Most Recently Updated

    std::list<BlockRequest> mRequests; // 0x0
    std::vector<Block *> mBlockCache; // 0x8
    Block *mReadingBlock; // 0x14
    Block *mWritingBlock; // 0x18
    Timer mSpinDownTimer; // 0x20
};

extern BlockMgr TheBlockMgr;
extern const int kArkBlockSize;
