#include "synth/StreamReceiver.h"
#include "StreamReceiver.h"
#include "os/Debug.h"
#include "xdk/xapilibi/xbox.h"
#include <cstring>

StreamReceiver::StreamReceiver(int numBuffers, bool slip)
    : mSlipEnabled(slip), mNumBuffers(numBuffers), mBuffer(), mRingFreeSpace(0),
      mState(kInit), mSendTarget(0), mWantToSend(false), mSending(false), mBuffersSent(0),
      mStarving(false), mEndData(false), mDoneBufferCounter(0), mLastPlayCursor(0) {
    MILO_ASSERT(numBuffers > 0, 0x33);
}

StreamReceiver::~StreamReceiver() {}

int StreamReceiver::BytesWriteable() { return kStreamRcvrBufSize - mRingFreeSpace; }
bool StreamReceiver::Ready() { return mState != kInit; }

void StreamReceiver::EndData() {
    if (!mEndData) {
        if (mRingFreeSpace < kStreamRcvrBufSize) {
            memset(&mBuffer[mRingFreeSpace], 0, kStreamRcvrBufSize - mRingFreeSpace);
            mRingFreeSpace = kStreamRcvrBufSize;
        }
        mEndData = true;
    }
}

void StreamReceiver::Play() {
    MILO_ASSERT(Ready(), 0x91);
    if (mState != kPlaying) {
        mState == kStopped ? PauseImpl(false) : PlayImpl();
        mState = kPlaying;
    }
}

void StreamReceiver::Stop() {
    MILO_ASSERT(mState == kPlaying || mState == kStopped, 0xA6);
    if (mState == kPlaying) {
        PauseImpl(true);
        mState = kStopped;
    }
}

StreamReceiver *StreamReceiver::New(int i1, int i2, bool b3, int i4) {
    MILO_ASSERT(sFactory, 0x1C);
    return sFactory(i1, i2, b3, i4);
}

void StreamReceiver::WriteData(void const *v, int bytes) {
    MILO_ASSERT(bytes > 0 && bytes <= BytesWriteable(), 0x51);
    XMemCpy(mBuffer + mRingFreeSpace, v, bytes);
    mRingFreeSpace += bytes;
}

u64 StreamReceiver::GetBytesPlayed() {
    if (mState == kInit) {
        return 0;
    }

    u64 numBuffers = mNumBuffers;
    u64 buffersSent = mBuffersSent;
    u64 bufferNum = buffersSent << 0xe;
    u64 val = (buffersSent / numBuffers) * numBuffers * 0x4000 + mLastPlayCursor;
    for (; val >= bufferNum; val = val - numBuffers * 0x4000) {
    }
    return val;
}

void StreamReceiver::Poll() {
    switch (mState) {
    case kInit:
        mWantToSend = true;
        break;
    case kReady:
        break;
    case kPlaying:
    case kStopped: {
        int playCursor = GetPlayCursor();
        int activeBuf = playCursor / 0x4000;
        mLastPlayCursor = playCursor;
        MILO_ASSERT(activeBuf >= 0 && activeBuf < mNumBuffers, 0xc2);
        if (!mSlipEnabled && activeBuf != mSendTarget) {
            mWantToSend = true;
        }
        if (activeBuf - mSendTarget == mNumBuffers / 2
            || activeBuf - mSendTarget == -(mNumBuffers / 2)) {
            mWantToSend = true;
        }
    } break;
    default:
        MILO_FAIL("bad state logic.\n");
        break;
    }
    if ((mWantToSend && Ready()) && BytesWriteable()) {
        mStarving = true;
    }

    if (mWantToSend && mRingFreeSpace >= 0x4000 && !mSending) {
        StartSendImpl(mBuffer, 0x4000, mSendTarget);
        mBuffersSent++;
        if (mBuffersSent >= 700000) {
            mBuffersSent -= mNumBuffers;
        }
        int target = mSendTarget;
        mWantToSend = false;
        mSending = true;
        mSendTarget++;
        if (target + 1 == mNumBuffers) {
            mSendTarget = 0;
        }
    }

    if (mSending && SendDoneImpl()) {
        mSending = false;
        mStarving = false;
        if (mSendTarget == 0 && !Ready()) {
            mState = kReady;
            mWantToSend = false;
        }
        int overflow = mRingFreeSpace - 0x4000;
        MILO_ASSERT(overflow >= 0, 0x134);
        if (overflow != 0) {
            XMemCpy(mBuffer, mBuffer + 0x4000, overflow);
        }
        mRingFreeSpace -= 0x4000;
        if (mEndData) {
            memset(&mBuffer[mRingFreeSpace], 0, kStreamRcvrBufSize - mRingFreeSpace);
            mRingFreeSpace = 0x8000;
            mDoneBufferCounter++;
        }
    }
}
