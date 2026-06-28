#include "utl/DeJitter.h"
#include "obj/Data.h"
#include "math/Utl.h"

float DeJitter::sTimeScale = 1;

DeJitter::DeJitter() { Reset(); }

void DeJitter::Reset() {
    mIndex = 0;
    mWindow = -2;
    mLastAverage = 0;
    mLastMs = 0;
    for (int i = 0; i < DIM(mBuffer); i++) {
        mBuffer[i] = 0;
    }
}

float DeJitter::NewMs(float ms, float &dt) {
    float f5 = kHugeFloat;
    static DataNode &n = DataVariable("dejitter_disable");
    int idxDiff = (mIndex - 1) & 0x1F;
    int idxDiff2 = (idxDiff - mWindow) & 0x1F;
    if (!n.Int() && mWindow > 8) {
        float diff = (mBuffer[idxDiff] - mBuffer[idxDiff2]) / (float)mWindow;
        if (mLastAverage == 0) {
            mLastAverage = diff;
        }
        mLastAverage = Interp(mLastAverage, diff, 0.1f);
        if (sTimeScale != 1) {
            mLastAverage *= sTimeScale;
            f5 = mLastAverage + mLastMs;
        } else {
            f5 = Clamp(ms - 33.0f, ms + 33.0f, mLastMs + mLastAverage);
        }
        if (f5 < mLastMs) {
            f5 = mLastMs;
        }
    }
    mBuffer[mIndex] = ms;
    if (f5 != kHugeFloat) {
        ms = f5;
    }
    mIndex = (mIndex + 1) & 0x1F;
    if (mWindow == -2) {
        dt = 16.666f;
    } else {
        dt = ms - mLastMs;
    }
    if (mWindow < 30) {
        mWindow++;
    }
    mLastMs = ms;
    return mLastMs;
}
