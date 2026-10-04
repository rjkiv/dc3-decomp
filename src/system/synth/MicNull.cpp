#include "synth/MicNull.h"
#include "math/Rand.h"
#include <cstring>

Rand sRand(0x1bca7);

short *MicNull::GetRecentBuf(int &size) {
    size = DIM(mRecentBuf);
    memcpy(mRecentBuf, mBuf, size * 2);
    return mRecentBuf;
}

MicNull::MicNull() {
    for (int i = 0; i < 10000U; i++) {
        mBuf[i] = sRand.Int(-32000, 32000);
    }
}

short *MicNull::GetContinuousBuf(int &size) {
    mTimer.Restart();
    size = (GetSampleRate() / 1000) * mTimer.GetLastMs();
    int val = size;
    if (val > 10000) {
        val = 10000;
    }
    size = val;
    int i = val % 8;
    if (i != 0) {
        size = val + i;
    }
    memcpy(mContinuousBuf, mBuf, size * 2);
    return mContinuousBuf;
}
