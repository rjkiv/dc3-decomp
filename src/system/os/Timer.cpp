#include "os/Timer.h"
#include "os/Debug.h"
#include "ppcintrinsics.h"

void Timer::SetLastMs(float ms) {
    mLastMs = ms;
    if (mLastMs >= mWorstMs) {
        mWorstMs = mLastMs;
        mWorstMsFrame = mFrame;
    }
}

TimerStats::TimerStats(DataArray *cfg)
    : mCount(0), mAvgMs(0.0f), mStdDevMs(0.0f), mMaxMs(0.0f), mNumOverBudget(0),
      mBudget(0.0f), mCritical(0), mNumCritOverBudget(0), mAvgMsInCrit(0.0f) {
    cfg->FindData("budget", mBudget, false);
    cfg->FindData("critical", mCritical, false);
    for (int i = 0; i < MAX_TOP_VALS; i++)
        mTopValues[i] = 0.0f;
}

void Timer::Init() {
    Timer::sDoubleCycles2Ms = 2.0050125313283208e-5;
    Timer::sLowCycles2Ms = 0.000020050125f;
    Timer::sHighCycles2Ms = 86114.63f;
}

Timer::Timer()
    : mStart(0), mCycles(0), mLastMs(0.0f), mWorstMs(0.0f), mWorstMsFrame(0), mFrame(0),
      mName(), mRunning(0), mBudget(0.0f), mDraw(true) {}

Timer::Timer(DataArray *config)
    : mStart(0), mCycles(0), mLastMs(0.0f), mWorstMs(0.0f), mWorstMsFrame(0), mFrame(0),
      mName(config->Sym(0)), mRunning(0), mBudget(0.0f), mDraw(true) {
    config->FindData("budget", mBudget, false);
    config->FindData("draw", mDraw, false);
}

void Timer::Restart() {
    unsigned long long cycle = __mftb();
    if (mRunning > 0) {
        mCycles += cycle - mStart;
    }

    Reset();

    mRunning = 1;
    mStart = cycle;
}

void TimerStats::CollectStats(float ms, bool critical, int critCount) {
    static int temp;
    if (mCount++ != 0) {
        mAvgMs += (ms - mAvgMs) / mCount;
        mStdDevMs += (fabs(ms - mAvgMs) - mStdDevMs) / mCount;

        if (ms > mMaxMs) {
            mMaxMs = ms;
        }

        if (mBudget > 0.0f && ms > mBudget) {
            mNumOverBudget++;
            if (critical) {
                mNumCritOverBudget++;
                mAvgMsInCrit += (ms - mAvgMsInCrit) / critCount;
            }
        }

        for (int i = 0; i < mCount && i < MAX_TOP_VALS; i++) {
            float top = mTopValues[i];
            if (ms > top) {
                mTopValues[i] = ms;
                ms = top;
            }
        }
    } else {
        mAvgMs = ms;
        mMaxMs = 0.0f;
        mStdDevMs = 0.0f;
    }
}

void TimerStats::PrintPctile(float pctile) {
    float target = mCount * (1.0f - pctile);
    float top;
    for (int i = 0; i < MAX_TOP_VALS; i++) {
        top = mTopValues[i];
        if (i > target) {
            break;
        }
    }

    int a = std::floor(pctile * 100);
    if (target > MAX_TOP_VALS) {
        MILO_LOG(
            "   %dth pctile:   <%.2f THIS IS AN OVERESTIMATE.  For accurate percentile, increase MAX_TOP_VALS in Timer.h\n",
            a,
            top
        );
    } else {
        MILO_LOG("   %dth pctile:   %.2f\n", a, top);
    }
}

void TimerStats::Dump(const char *tag, int critCount) {
    if (mCount != 0) {
        MILO_LOG(
            "%s\t(%2.2f, %2.2f), %4.2f, [>%.1f] %.2f {%2.2f} %.1f\n",
            tag,
            mAvgMs,
            mStdDevMs,
            mMaxMs,
            mBudget,
            (float)(mNumOverBudget * 100) / mCount,
            mAvgMsInCrit,
            (float)(mNumCritOverBudget * 100) / critCount
        );
        PrintPctile(0.99);
    } else {
        MILO_LOG("%s <no data>\n", tag);
    }
}
