#include "CDReader.h"
#include "obj/DataFunc.h"
#include "os/AsyncTask.h"
#include "os/Block.h"
#include "os/Debug.h"
#include "os/HDCache.h"
#include "os/System.h"
#include "utl/MemMgr.h"

#define kNumBlockBuffers 0x13

static char *gBuffers;
int gCurrBuffNum;
int Block::sCurrTimestamp;
Timer gReadTime;

BlockMgr TheBlockMgr;

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
      mStr(task.GetStr()) {
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
        if (mBlockCache[i]->CheckMetadata(arknum, blocknum))
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
    int arkfileNum = task.GetArkfileNum();
    FOREACH (it, mRequests) {
        if (it->CheckMetadata(arkfileNum, task.GetBlockNum())) {
            it->mTasks.push_back(task);
            return;
        }
        if (it->LessThan(arkfileNum, task.GetBlockNum())) {
            mRequests.push_back(BlockRequest(task));
            it->mTasks.clear();
            return;
        }
    }
    mRequests.push_back(BlockRequest(task));
}

bool BlockMgr::SpinUp() {
    TheBlockMgr.Poll();
    if (UsingCD() && mSpinDownTimer.Ms() > 120000.0f) {
        if (!mReadingBlock) {
            MILO_LOG("BlockMgr spinning up...\n");
            mReadingBlock = FindMRUBlock();
            AsyncTask task(mReadingBlock->mArkfileNum, mReadingBlock->mBlockNum);
            AddTask(task);
            gReadHD = false;
            int err = CDRead(
                mReadingBlock->mArkfileNum, mReadingBlock->mBlockNum << 5, 2, gTempBlock
            );
            bool good = err == 1;
            if (good) {
                mReadingBlock->UpdateTimestamp();
            } else {
                MILO_LOG("CD READING ERROR: %x\n", err);
                mReadingBlock = nullptr;
            }
        }
        return false;
    }
    return true;
}
