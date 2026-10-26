#include "utl/MultiTempoTempoMap.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include <cmath>
#include <algorithm>

MultiTempoTempoMap::MultiTempoTempoMap() : mStartLoopTick(-1.0f), mEndLoopTick(-1.0f) {}

bool MultiTempoTempoMap::CompareTick(
    float tick, const MultiTempoTempoMap::TempoInfoPoint &pt
) {
    return tick < pt.mTick;
}

bool MultiTempoTempoMap::CompareTime(
    float time, const MultiTempoTempoMap::TempoInfoPoint &pt
) {
    return time < pt.mMs;
}

float MultiTempoTempoMap::GetTempoBPM(int tick) const {
    return 60000.0f / GetTempo(tick);
}

void MultiTempoTempoMap::ClearLoopPoints() {
    mStartLoopTick = -1.0f;
    mEndLoopTick = -1.0f;
    mStartLoopTime = -1.0f;
    mEndLoopTime = -1.0f;
}

void MultiTempoTempoMap::SetLoopPoints(int start, int end) {
    mStartLoopTick = start;
    mEndLoopTick = end;
    mStartLoopTime = TickToTime(mStartLoopTick);
    mEndLoopTime = TickToTime(mEndLoopTick);
}

int MultiTempoTempoMap::GetLoopTick(int tick, int &tickOffset) const {
    if (mStartLoopTick < 0) {
        return tick;
    } else {
        tickOffset = 0;
        int startTick = mStartLoopTick;
        int endTick = mEndLoopTick;

        if (tick >= mEndLoopTick && mStartLoopTick != mEndLoopTick) {
            int loopTick = tick - startTick;
            int loopLength = endTick - startTick;
            int newTick = (loopTick % loopLength) + startTick;
            tickOffset = tick - newTick;
            return newTick;
        }
        return tick;
    }
}

int MultiTempoTempoMap::GetLoopTick(int tick) const {
    int ok;
    return GetLoopTick(tick, ok);
}

float MultiTempoTempoMap::GetTimeInLoop(float time) {
    if (mStartLoopTick == -1.0f) {
        return time;
    }

    float startTime = TickToTime(mStartLoopTick);
    if (time < startTime) {
        return time;
    }

    float endTime = TickToTime(mEndLoopTick);

    float loopLength = endTime - startTime;
    float timeFromStart = time - startTime;
    MILO_ASSERT(timeFromStart >= 0.0f, 0xE3);

    float a = floorf(timeFromStart / loopLength);
    float ret = -(loopLength * a - timeFromStart);
    return startTime + ret;
}

int MultiTempoTempoMap::GetNumTempoChangePoints() const { return mTempoPoints.size(); }

int MultiTempoTempoMap::GetTempoChangePoint(int index) const {
    MILO_ASSERT(index < mTempoPoints.size(), 0xF7);
    return mTempoPoints[index].mTick;
}

const MultiTempoTempoMap::TempoInfoPoint *
MultiTempoTempoMap::PointForTick(float tick) const {
    if (mTempoPoints.size() < 1) {
        MILO_NOTIFY("Tempo map is empty; at least one tempo map entry is required");
        return mTempoPoints.end();
    } else {
        const TempoInfoPoint *pt2 =
            std::upper_bound(mTempoPoints.begin(), mTempoPoints.end(), tick, CompareTick);
        if (pt2 != mTempoPoints.begin()) {
            pt2--;
        }
        return pt2;
    }
}

const MultiTempoTempoMap::TempoInfoPoint *
MultiTempoTempoMap::PointForTime(float time) const {
    MILO_ASSERT(mTempoPoints.size() >= 1, 0x121);
    const TempoInfoPoint *pt2 =
        std::upper_bound(mTempoPoints.begin(), mTempoPoints.end(), time, CompareTime);
    if (pt2 != mTempoPoints.begin()) {
        pt2--;
    }
    return pt2;
}

float MultiTempoTempoMap::GetTempo(int tick) const {
    const TempoInfoPoint *pt = PointForTick(tick);
    if (pt != mTempoPoints.end())
        return (float)pt->mTempo / 1000.0f;
    else
        return 800.0f;
}

int MultiTempoTempoMap::GetTempoInMicroseconds(int tick) const {
    const TempoInfoPoint *pt = PointForTick(tick);
    if (pt != mTempoPoints.end())
        return pt->mTempo;
    else
        return 800000;
}

float MultiTempoTempoMap::TickToTime(float tick) const {
    if (tick == 0) {
        return 0;
    }

    if (mStartLoopTick >= 0 && tick > mEndLoopTick) {
        float tickDiff = mEndLoopTick - mStartLoopTick;
        float timeDiff = tick - mEndLoopTick;
        float loopTimeDiff = mEndLoopTime - mStartLoopTime;

        float loopPct = floorf(timeDiff / tickDiff);

        float f4 = loopPct * loopTimeDiff + mEndLoopTime;

        f4 += TickToTime(timeDiff - loopPct * tickDiff + mStartLoopTick) - mStartLoopTime;
        return f4;
    } else {
        const TempoInfoPoint *pt = PointForTick(tick);
        if (pt == mTempoPoints.end()) {
            return 0;
        } else {
            return (tick - pt->mTick) * (float)pt->mTempo / 480000 + pt->mMs;
        }
    }
}

float MultiTempoTempoMap::TimeToTick(float time) const {
    if (time != 0) {
        if (mStartLoopTick >= 0 && mEndLoopTick >= 0 && time > mEndLoopTime) {
            float tickDiff = mEndLoopTick - mStartLoopTick;
            float timeDiff = time - mEndLoopTime;
            float loopTimeDiff = mEndLoopTime - mStartLoopTime;

            float loopPct = floorf(timeDiff / loopTimeDiff);
            float f5 = loopPct * tickDiff + mEndLoopTick;
            float f3 = timeDiff - loopPct * loopTimeDiff + mStartLoopTime;

            f5 += TimeToTick(f3) - mStartLoopTick;
            return f5;
        } else {
            const TempoInfoPoint *pt = PointForTime(time);
            return pt->mTick + ((time - pt->mMs) * 1000.0f / (float)pt->mTempo) * 480.0f;
        }
    } else {
        return 0;
    }
}

void MultiTempoTempoMap::Finalize() { TrimExcess(mTempoPoints); }

bool MultiTempoTempoMap::AddTempoInfoPoint(int tick, int tempo) {
    if (mTempoPoints.size() == 0) {
        if (tick != 0) {
            return false;
        }
    } else if (tick < mTempoPoints.back().mTick) {
        return false;
    }

    MemDoTempAllocations tmp;
    mTempoPoints.push_back(TempoInfoPoint(TickToTime(tick), tick, tempo));
    return true;
}
