#include "synth/StreamReceiverFile.h"
#include "StreamReceiver.h"
#include "StreamReceiverFile.h"
#include "os/Debug.h"

StreamReceiverFile::StreamReceiverFile(int numBuffers, bool slip)
    : StreamReceiver(numBuffers, slip), mTargetBuffer(nullptr), mBufSize(0) {}

void StreamReceiverFile::StartSendImpl(unsigned char *data, int bufSize, int targetIdx) {
    MILO_ASSERT(mTargetBuffer, 0x1D);
    MILO_ASSERT(targetIdx * kStreamBufSize + bufSize <= mBufSize, 0x1F);
    memcpy(targetIdx * kStreamBufSize + mTargetBuffer, data, bufSize);
}

int StreamReceiverFile::GetPlayCursor() {
    return ((unsigned int)sPlayCursor * 2) % (mNumBuffers * kStreamBufSize);
}
