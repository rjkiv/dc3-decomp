#include "CDReader.h"
#include "obj/DataFunc.h"
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
    bool err = false;
    char *buf = (char *)mReadingBlock->Buffer();
    int arkNum = mReadingBlock->ArkFileNum();
    int blockNum = mReadingBlock->BlockNum();
    if (TheHDCache.ReadAsync(arkNum, blockNum, buf)) {
        gReadHD = true;
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
        block->SetWritten();
        if (TheHDCache.WriteAsync(
                block->ArkFileNum(), block->BlockNum(), block->Buffer()
            )) {
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
    int time = Block::CurrentTimestamp();
    Block *ret = nullptr;
    for (int i = 0; i < mBlockCache.size(); i++) {
        if (mBlockCache[i] != mWritingBlock && mBlockCache[i] != mReadingBlock
            && (!b || !mBlockCache[i]->Written() && mBlockCache[i]->Timestamp() < time)) {
            ret = mBlockCache[i];
            time = mBlockCache[i]->Timestamp();
        }
    }
    return ret;
}

Block *BlockMgr::FindMRUBlock() {
    int time = -1;
    Block *ret = nullptr;
    for (int i = 0; i < mBlockCache.size(); i++) {
        if (mBlockCache[i]->Timestamp() > time) {
            ret = mBlockCache[i];
            time = mBlockCache[i]->Timestamp();
        }
    }
    return ret;
}

char *BlockMgr::GetBlockData(int ark, int blk) {
    Block *blokc = FindBlock(ark, blk);
    if (blokc != nullptr && blokc != mReadingBlock) {
        blokc->UpdateTimestamp();
        return (char *)blokc->Buffer();
    }
    return nullptr;
}
