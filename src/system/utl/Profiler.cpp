#include "utl/Profiler.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include <float.h>

Profiler::Profiler(char const *c, int i)
    : mName(c), mMin(FLT_MAX), mMax(0.0f), mSum(0.0f), mCount(0), mCountMax(i) {}

void Profiler::Start() { mTimer.Start(); }

void Profiler::Stop() {
    mTimer.Stop();
    MinEq(mMin, mTimer.Ms());
    MaxEq(mMax, mTimer.Ms());
    mSum += mTimer.Ms();
    if (++mCount == mCountMax) {
        if (mCountMax == 1) {
            MILO_LOG("%s: %s\n", mName, FormatTime(mMin));
        } else {
            MILO_LOG(
                "%s: min %s max %s mean %s\n",
                mName,
                FormatTime(mMin),
                FormatTime(mMax),
                FormatTime(mSum / mCount)
            );
        }
        mCount = 0;
        mMin = FLT_MAX;
        mMax = 0;
        mSum = 0;
    }
    mTimer.Reset();
}
