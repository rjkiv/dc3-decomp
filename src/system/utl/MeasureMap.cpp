#include "utl/MeasureMap.h"
#include "os/Debug.h"
#include <algorithm>

bool MeasureMap::CompareTick(int tick, const TimeSigChange &sig) {
    return tick < sig.mTick;
}

bool MeasureMap::CompareMeasure(int measure, const TimeSigChange &sig) {
    return measure < sig.mMeasure;
}

int MeasureMap::MeasureBeatTickToTick(int measure, int beat, int tick) const {
    const TimeSigChange *change = std::upper_bound(
        mTimeSigChanges.begin(), mTimeSigChanges.end(), measure, CompareMeasure
    );
    if (change != mTimeSigChanges.begin())
        change--;
    return ((measure - change->mMeasure) * change->mNum * 1920) / change->mDenom
        + change->mTick + beat * 480 + tick;
}

void MeasureMap::TickToMeasureBeatTick(
    int tick, int &oMeasure, int &oBeat, int &oTick, int &oBeatsPerMeasure
) const {
    const TimeSigChange *change = std::upper_bound(
        mTimeSigChanges.begin(), mTimeSigChanges.end(), tick, CompareTick
    );
    if (change != mTimeSigChanges.begin())
        change--;
    int div = (change->mNum * 1920) / change->mDenom;
    int tickDiff = tick - change->mTick;
    int tickDiv = tickDiff / div;
    int mod = tickDiff - tickDiv * div;
    oMeasure = change->mMeasure + tickDiv;
    int modBeat = mod / 480;
    oBeat = modBeat;
    oTick = mod - modBeat * 480;
    oBeatsPerMeasure = div / 480;
}

void MeasureMap::TickToMeasureBeatTick(
    int tick, int &oMeasure, int &oBeat, int &oTick
) const {
    int bpm;
    TickToMeasureBeatTick(tick, oMeasure, oBeat, oTick, bpm);
}

MeasureMap::MeasureMap() : mTimeSigChanges() {
    mTimeSigChanges.push_back(TimeSigChange());
}

bool MeasureMap::AddTimeSignature(int measure, int num, int denom, bool fail) {
    if (measure == 0) {
        if (mTimeSigChanges.size() != 1) {
            if (fail) {
                MILO_FAIL("Multiple time signatures at start of song");
            } else {
                return false;
            }
        }
        mTimeSigChanges.pop_back();
        mTimeSigChanges.push_back(TimeSigChange(0, num, denom, 0));
    } else {
        TimeSigChange &sig = mTimeSigChanges.back();
        int measureDiff = measure - sig.mMeasure;
        if (measureDiff <= 0) {
            if (fail) {
                MILO_FAIL("Multiple time signatures at measure %d", measure);
            } else {
                return false;
            }
        }
        mTimeSigChanges.push_back(TimeSigChange(
            measure, num, denom, sig.mTick + (sig.mNum * measureDiff * 1920) / sig.mDenom
        ));
    }
    return true;
}
