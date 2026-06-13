#include "os/Timer.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ppcintrinsics.h"
#include "xdk/XAPILIB.h"
#include "math/Utl.h"
#include "utl/Std.h"

bool AutoTimer::sCriticalFrame;
bool AutoTimer::sCollectingStats;
bool gGlitchCallback;
int AutoTimer::sCritFrameCount;
float Timer::sLowCycles2Ms;
float Timer::sHighCycles2Ms;
double Timer::sDoubleCycles2Ms;
float Timer::sSlowFrameWaiver;
int AutoSlowFrame::sDepth;
const char *Timer::sSlowFrameReason;
int AutoGlitchReport::sDepth;
Timer Timer::sSlowFrameTimer;
std::list<std::pair<Timer, TimerStats> > AutoTimer::sTimers;
std::list<Symbol> sConditionalTimersEnabled;

#pragma region Timer

Timer::Timer()
    : mStart(0), mCycles(0), mLastMs(0.0f), mWorstMs(0.0f), mWorstMsFrame(0), mFrame(0),
      mName(), mRunning(0), mBudget(0.0f), mDraw(true) {}

Timer::Timer(DataArray *config)
    : mStart(0), mCycles(0), mLastMs(0.0f), mWorstMs(0.0f), mWorstMsFrame(0), mFrame(0),
      mName(config->Sym(0)), mRunning(0), mBudget(0.0f), mDraw(true) {
    config->FindData("budget", mBudget, false);
    config->FindData("draw", mDraw, false);
}

void Timer::Sleep(int ms) { ::Sleep(ms); }

void Timer::SetLastMs(float ms) {
    mLastMs = ms;
    if (mLastMs >= mWorstMs) {
        mWorstMs = mLastMs;
        mWorstMsFrame = mFrame;
    }
}

void Timer::Init() {
    Timer::sDoubleCycles2Ms = 2.0050125313283208e-5;
    Timer::sLowCycles2Ms = 0.000020050125f;
    Timer::sHighCycles2Ms = 86114.63f;
}

void Timer::Reset() {
    SetLastMs(Ms());
    mFrame++;
    mCycles = 0;
    mRunning = 0;
    if (mFrame > mWorstMsFrame + 60.0f) {
        mWorstMsFrame = 0;
        mWorstMs = 0;
    }
}

void Timer::Restart() {
    unsigned int cycle = __mftb();
    if (mRunning > 0) {
        mCycles += cycle - mStart;
    }
    Reset();
    mRunning = 1;
    mStart = cycle;
}

#pragma endregion
#pragma region TimerStats

TimerStats::TimerStats(DataArray *cfg)
    : mCount(0), mAvgMs(0.0f), mStdDevMs(0.0f), mMaxMs(0.0f), mNumOverBudget(0),
      mBudget(0.0f), mCritical(0), mNumCritOverBudget(0), mAvgMsInCrit(0.0f) {
    cfg->FindData("budget", mBudget, false);
    cfg->FindData("critical", mCritical, false);
    for (int i = 0; i < MAX_TOP_VALS; i++)
        mTopValues[i] = 0.0f;
}

void TimerStats::CollectStats(float ms, bool critical, int critCount) {
    static float sTopMs;
    if (mCount++ != 0) {
        mAvgMs += (ms - mAvgMs) / mCount;
        float diff = ms - mAvgMs;
        if (diff <= 0) {
            diff = -diff;
        }
        mStdDevMs += (diff - mStdDevMs) / mCount;
        if (ms > mMaxMs) {
            mMaxMs = ms;
        }
        if (mBudget > 0 && ms > mBudget) {
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
                sTopMs = top;
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

void TimerStats::Clear() {
    mCount = 0;
    mAvgMs = 0;
    mStdDevMs = 0;
    mMaxMs = 0;
    mNumOverBudget = 0;
    mNumCritOverBudget = 0;
    mAvgMsInCrit = 0;
    for (int i = 0; i < MAX_TOP_VALS; i++) {
        mTopValues[i] = 0;
    }
}

#pragma endregion
#pragma region Handlers

DataNode ShowTimer(DataArray *arr) {
    Timer *timer = AutoTimer::GetTimer(arr->Sym(1));
    if (timer) {
        timer->SetDraw(arr->Int(2));
    }
    return 0;
}

DataNode SetTimerMs(DataArray *arr) {
    Timer *timer = AutoTimer::GetTimer(arr->Sym(1));
    if (timer) {
        timer->SetLastMs(arr->Float(2));
    }
    return 0;
}

DataNode TimerMs(DataArray *arr) {
    Timer *timer = AutoTimer::GetTimer(arr->Sym(1));
    if (timer)
        return timer->GetLastMs();
    else
        return 0;
}

DataNode OnPrintTimers(DataArray *a) {
    AutoTimer::PrintTimers(a->Int(1));
    return 0;
}

#pragma endregion
#pragma region AutoGlitchReport

void AutoGlitchReport::EnableCallback() { gGlitchCallback = true; }

void AutoGlitchReport::SendCallback(
    float f1, float f2, const char *cc, AutoTimerCallback cb, void *v
) {
    if (gGlitchCallback) {
        float min = Min(Timer::SlowFrameTimer().SplitMs(), Timer::SlowFrameWaiver());
        float diff = f1 - min;
        if (diff >= f2) {
            String str;
            for (int i = 0; i < sDepth; i++) {
                str += ' ';
            }
            TheDebug.Print(str.c_str());
            if (!cb) {
                MILO_LOG("%s took %.2f ms\n", cc, diff);
            } else {
                cb(diff, v);
            }
        }
    }
}

#pragma endregion
#pragma region AutoTimer

Timer *AutoTimer::GetTimer(Symbol name) {
    FOREACH (it, sTimers) {
        Timer &timer = it->first;
        if (timer.Name() == name) {
            return &timer;
        }
    }
    return nullptr;
}

bool AutoTimer::CollectingStats() { return sCollectingStats; }

void AutoTimer::ComputeCriticalFrame() {
    sCriticalFrame = false;
    FOREACH (it, sTimers) {
        if (it->second.mCritical && it->first.GetLastMs() > it->first.Budget()) {
            sCriticalFrame = true;
            sCritFrameCount++;
            break;
        }
    }
}

void AutoTimer::CollectTimerStats() {
    if (sCollectingStats) {
        ComputeCriticalFrame();
        FOREACH (it, sTimers) {
            if (it->first.Draw()) {
                it->second.CollectStats(
                    it->first.GetLastMs(), sCriticalFrame, sCritFrameCount
                );
            }
        }
    }
}

void AutoTimer::DumpTimerStats() {
    if (sCritFrameCount == 0) {
        sCritFrameCount = 1;
    }
    MILO_LOG(
        "SONG TIMER STATS: (mean, SD), max, [>budget] pct overbudget {mean in critical frames} pct of crit frames overbudget\n"
    );
    FOREACH (it, sTimers) {
        TimerStats &stats = it->second;
        if (it->first.Draw()) {
            stats.Dump(it->first.Name().Str(), sCritFrameCount);
        }
        stats.Clear();
    }
    sCritFrameCount = 0;
}

void AutoTimer::PrintTimers(bool worst) {
    MILO_LOG("%s TIMES:\n", worst ? "WORST" : "LAST");
    FOREACH (it, sTimers) {
        if (it->first.Draw()) {
            MILO_LOG(
                "%s %s\n",
                it->first.Name(),
                FormatTime(worst ? it->first.GetWorstMs() : it->first.GetLastMs())
            );
        }
    }
}

void AutoTimer::SetCollectStats(bool collect, bool dump) {
    sCollectingStats = collect;
    if (dump && !collect) {
        DumpTimerStats();
    }
}

void AutoTimer::Init() {
    DataRegisterFunc("show_timer", ShowTimer);
    DataRegisterFunc("set_timer_ms", SetTimerMs);
    DataRegisterFunc("timer_ms", TimerMs);
    DataRegisterFunc("print_timers", OnPrintTimers);
    DataArray *cfg = SystemConfig("timer");
    for (int i = 1; i < cfg->Size(); i++) {
        DataArray *arr = cfg->Array(i);
        bool enabled = false;
        arr->FindData("enable", enabled, false);
        if (enabled) {
            sTimers.push_back(std::pair<Timer, TimerStats>(Timer(arr), TimerStats(arr)));
        }
    }
}
