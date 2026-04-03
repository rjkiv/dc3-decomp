#pragma once
#include "os/AsyncTask.h"
#include "os/Timer.h"
#include "utl/PoolAlloc.h"

class Block {
public:
    Block();
    void UpdateTimestamp();

    bool CheckMetadata(int arknum, int blocknum) const {
        return mArkfileNum == arknum && mBlockNum == blocknum;
    }

    MEM_OVERLOAD(Block, 0x16);
    static int CurrentTimestamp() { return sCurrTimestamp; }

    // RB2 says it's all public
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

    bool CheckMetadata(int arknum, int blocknum) const {
        return mArkfileNum == arknum && mBlockNum == blocknum;
    }

    bool LessThan(int arknum, int blocknum) const {
        return (arknum < mArkfileNum) || (mArkfileNum == arknum && mBlockNum > blocknum);
    }

    // same
    int mArkfileNum; // 0x0
    int mBlockNum; // 0x4
    const char *mStr; // 0x8
    std::list<AsyncTask> mTasks; // 0xc
};

class BlockMgr {
public:
    BlockMgr() {}
    ~BlockMgr() {}
    char *GetBlockData(int, int);
    void KillBlockRequests(ArkFile *);
    void Poll();
    void GetAssociatedBlocks(unsigned long long, int, int &, int &, int &);
    void AddTask(const AsyncTask &);
    bool SpinUp();
    void Init();
    void MarkDiscRead();

private:
    void WriteBlock();
    void ReadBlock();
    Block *FindBlock(int, int);
    Block *FindLRUBlock(bool); // Least Recently Updated
    Block *FindMRUBlock(); // Most Recently Updated

    std::list<BlockRequest> mRequests; // 0x0
    std::vector<Block *> mBlockCache; // 0x8
    Block *mReadingBlock; // 0x14
    Block *mWritingBlock; // 0x18
    Timer mSpinDownTimer; // 0x20
};

extern BlockMgr TheBlockMgr;
