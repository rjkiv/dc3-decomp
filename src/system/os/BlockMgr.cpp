#include "CDReader.h"
#include "obj/DataFunc.h"
#include "os/AsyncTask.h"
#include "os/Block.h"
#include "os/Debug.h"
#include "os/HDCache.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "utl/MemMgr.h"

#define kNumBlockBuffers 0x13

// static bool gReadHD; // RVA=0x1098f58
// static char* gTempBlock; // RVA=0x1098f5c
// static char gBuffers[1245184]; // RVA=0xf68f58
// static int32_t gNumPolls; // RVA=0x1098f68
// static Timer gReadTime; // RVA=0x1098fc8
// static void (* gReadTime$initializer$)(); // RVA=0xf05d7c
// static int32_t gLastBlockNum; // RVA=0xf12620
// static int32_t gLastArkNum; // RVA=0xf12624
// static int32_t gSeekCount; // RVA=0x1098f6c
// static float gSeekTimeMs; // RVA=0x1098f70

static char gBuffers[0x10000 * kNumBlockBuffers];

namespace {
    bool gReadHD = false;
    char *gTempBlock;

    int ReadError() {
        if (gReadHD) {
            return TheHDCache.ReadFail();
        } else {
            return CDGetError();
        }
    }

    static DataNode OnSpinUp(DataArray *) { return TheBlockMgr.SpinUp(); }
}

int gCurrBuffNum;
int Block::sCurrTimestamp;
Timer gReadTime;
BlockMgr TheBlockMgr;

static int gNumPolls;
static int gSeekCount;
static float gSeekTimeMs;

static const float kSeekTimeMsThresh = 240;
static const float kSpinDownMs = 120000;

int GetFreeBuffer() {
    MILO_ASSERT(gCurrBuffNum < kNumBlockBuffers, 0x46);
    return gCurrBuffNum++;
}

Block::Block()
    : mBuffer(0), mArkfileNum(-1), mBlockNum(-1), mTimestamp(-1), mWritten(true),
      mDebugName("") {
    mBuffer = &gBuffers[GetFreeBuffer() * 0x10000];
    UpdateTimestamp();
}

void Block::UpdateTimestamp() { mTimestamp = ++sCurrTimestamp; }

BlockRequest::BlockRequest(const AsyncTask &task)
    : mArkfileNum(task.GetArkfileNum()), mBlockNum(task.GetBlockNum()),
      mDebugName(task.GetDebugName()) {
    mTasks.push_back(task);
}

void BlockMgr::Init() {
    gTempBlock = (char *)MemAlloc(0x1000, __FILE__, 0xB7, "BlockMgr junk", 4);
    gCurrBuffNum = 0;
    mBlockCache.resize(kNumBlockBuffers);
    mReadingBlock = nullptr;
    for (int i = 0; i < mBlockCache.size(); i++) {
        mBlockCache[i] = new Block();
    }
    TheHDCache.Init();
    DataRegisterFunc("disc_spin_up", OnSpinUp);
}

void BlockMgr::ReadBlock() {
    MILO_ASSERT(mReadingBlock, 0x174);
    char *buf = mReadingBlock->mBuffer;
    int arkNum = mReadingBlock->mArkfileNum;
    int blockNum = mReadingBlock->mBlockNum;
    bool async = TheHDCache.ReadAsync(arkNum, blockNum, buf);
    bool err;
    if (async) {
        gReadHD = true;
        err = false;
    } else {
        gReadHD = false;
        err = CDRead(arkNum, blockNum << 5, 32, buf);
    }
    if (!err) {
        mReadingBlock->UpdateTimestamp();
    } else {
        MILO_LOG("CD READING ERROR: %x\n", err);
        mReadingBlock = nullptr;
    }
}

void BlockMgr::MarkDiscRead() { mSpinDownTimer.Restart(); }

void BlockMgr::WriteBlock() {
    MILO_ASSERT(!mWritingBlock, 0x161);
    for (Block *block = FindLRUBlock(true); block != nullptr;
         block = FindLRUBlock(true)) {
        block->mWritten = true;
        if (TheHDCache.WriteAsync(block->mArkfileNum, block->mBlockNum, block->mBuffer)) {
            mWritingBlock = block;
            return;
        }
    }
}

Block *BlockMgr::FindBlock(int arknum, int blocknum) {
    for (int i = 0; i < mBlockCache.size(); i++) {
        if (mBlockCache[i]->Matches(arknum, blocknum))
            return mBlockCache[i];
    }
    return nullptr;
}

Block *BlockMgr::FindLRUBlock(bool b) {
    int minTime = Block::CurrentTimestamp();
    Block *lruBlock = nullptr;
    for (int i = 0; i < mBlockCache.size(); i++) {
        if (mBlockCache[i] != mWritingBlock && mBlockCache[i] != mReadingBlock) {
            if (b && mBlockCache[i]->mWritten) {
                continue;
            }
            if (mBlockCache[i]->mTimestamp < minTime) {
                lruBlock = mBlockCache[i];
                minTime = mBlockCache[i]->mTimestamp;
            }
        }
    }
    return lruBlock;
}

Block *BlockMgr::FindMRUBlock() {
    int maxTime = -1;
    Block *mruBlock = nullptr;
    for (int i = 0; i < mBlockCache.size(); i++) {
        if (mBlockCache[i]->mTimestamp > maxTime) {
            mruBlock = mBlockCache[i];
            maxTime = mBlockCache[i]->mTimestamp;
        }
    }
    return mruBlock;
}

char *BlockMgr::GetBlockData(int ark, int blk) {
    Block *blokc = FindBlock(ark, blk);
    if (blokc != nullptr && blokc != mReadingBlock) {
        blokc->UpdateTimestamp();
        return blokc->mBuffer;
    }
    return nullptr;
}

void BlockMgr::AddTask(const AsyncTask &task) {
    std::list<BlockRequest>::iterator it;
    int blockNum = task.GetBlockNum();
    int arkfileNum = task.GetArkfileNum();
    for (it = mRequests.begin(); it != mRequests.end(); ++it) {
        if (it->Matches(arkfileNum, blockNum)) {
            it->mTasks.push_back(task);
            break;
        }
        if (it->LessThan(arkfileNum, blockNum)) {
            mRequests.insert(it, BlockRequest(task));
            break;
        }
    }
    if (it == mRequests.end()) {
        mRequests.push_back(BlockRequest(task));
    }
}

bool BlockMgr::SpinUp() {
    TheBlockMgr.Poll();
    if (UsingCD() && mSpinDownTimer.Ms() > kSpinDownMs) {
        if (!mReadingBlock) {
            MILO_LOG("BlockMgr spinning up...\n");
            mReadingBlock = FindMRUBlock();
            AsyncTask task(mReadingBlock->mArkfileNum, mReadingBlock->mBlockNum);
            AddTask(task);
            gReadHD = false;
            bool error = CDRead(
                mReadingBlock->mArkfileNum, mReadingBlock->mBlockNum << 5, 2, gTempBlock
            );
            if (!error) {
                mReadingBlock->UpdateTimestamp();
            } else {
                // this would only ever print 1 lol
                MILO_LOG("CD READING ERROR: %x\n", error);
                mReadingBlock = nullptr;
            }
        }
        return false;
    }
    return true;
}

void BlockMgr::GetAssociatedBlocks(
    u64 byteOffset, int numBytes, int &oStartBlock, int &oNumBlocks, int &oBlockSize
) {
    oBlockSize = 0x10000;
    oStartBlock = byteOffset >> 16;
    int i1 = (byteOffset & 0xFFFF) + numBytes - 0x10000;
    if (i1 > 0) {
        oNumBlocks = i1 / 0x10000 + 1;
        if (i1 % 0x10000) {
            oNumBlocks++;
        }
    } else {
        oNumBlocks = 1;
    }
}

void BlockMgr::KillBlockRequests(ArkFile *iArk) {
    for (auto it = mRequests.begin(); it != mRequests.end();) {
        for (auto taskIt = it->mTasks.begin(); taskIt != it->mTasks.end();) {
            if (taskIt->IsOwnedBy(iArk)) {
                taskIt = it->mTasks.erase(taskIt);
            } else {
                ++taskIt;
            }
        }
        if (it->mTasks.size() == 0) {
            if (mReadingBlock && mReadingBlock->Matches(it->mArkfileNum, it->mBlockNum)) {
                ++it;
            } else {
                it = mRequests.erase(it);
            }
        } else {
            ++it;
        }
    }
}

void BlockMgr::Poll() {
    if (MainThread()) {
        TheHDCache.Poll();
        mSpinDownTimer.Split();
        if (mWritingBlock && TheHDCache.WriteDone()) {
            mWritingBlock = nullptr;
            WriteBlock();
        }
        if (mReadingBlock) {
            gNumPolls++;
        }
    }
}
