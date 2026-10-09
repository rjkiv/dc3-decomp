#include "os/AsyncTask.h"
#include "os/ArkFile_p.h"
#include "os/Block.h"
#include "os/Debug.h"
#include <cstring>

AsyncTask::AsyncTask(
    ArkFile *owner, void *iBuffer, int arkfileNum, int iBlockNum, int iOffsetStart, int iOffsetEnd, const char *debugName
)
    : mArkfileNum(arkfileNum), mBlockNum(iBlockNum), mOffsetStart(iOffsetStart), mOffsetEnd(iOffsetEnd),
      mBuffer(iBuffer), mStr(debugName), mOwner(owner) {
    MILO_ASSERT(mOwner, 0x1B);
}

AsyncTask::AsyncTask(int arkfileNum, int blockNum)
    : mArkfileNum(arkfileNum), mBlockNum(blockNum), mOffsetStart(-1), mOffsetEnd(-1), mBuffer(0),
      mStr(gNullStr), mOwner(0) {}

bool AsyncTask::FillData() {
    char *data = TheBlockMgr.GetBlockData(mArkfileNum, mBlockNum);
    if (data && mOwner) {
        memcpy(mBuffer, &data[mOffsetStart], mOffsetEnd - mOffsetStart);
        mOwner->TaskDone(mOffsetEnd - mOffsetStart);
        return true;
    }
    return false;
}
