#include "rndobj/Wind.h"

#include "utl/BinStream.h"

float gUnitsPerMeter = 39.370079f;

void RndWind::SyncLoops() {
    float f1 = (mTimeLoop == 0.0f) ? 0.0f : 1.0f / mTimeLoop;
    mTimeRate.Set(f1, f1 * 0.773437f, f1 * 1.38484f);
    f1 = (mSpaceLoop == 0.0f) ? 0.0f : 1.0f / mSpaceLoop;
    mSpaceRate.Set(f1, f1 * 0.773437f, f1 * 1.38484f);
}

RndWind::~RndWind() {}

void RndWind::Save(BinStream &bs) {
    bs << 4;
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPrevailing;
    bs << mRandom;
    bs << mTimeLoop;
    bs << mSpaceLoop;
    bs << mWindOwner;
    bs << mTrans;
    bs << mAboutZ;
    bs << mMinSpeed;
    bs << mMaxSpeed;
}

RndWind::RndWind()
    : mPrevailing(0, 0, 0), mRandom(0, 0, 0), mTrans(this), mWindOwner(this) {
    SyncLoops();
}

BEGIN_PROPSYNCS(RndWind)
    SYNC_PROP(prevailing, mPrevailing)
    SYNC_PROP(random, mRandom)
    SYNC_PROP(max_speed, mMaxSpeed)
    SYNC_PROP(min_speed, mMinSpeed)
    SYNC_PROP_SET(wind_owner, mWindOwner.Ptr(), SetWindOwner(_val.Obj<RndWind>()))
    SYNC_PROP_MODIFY(time_loop, mTimeLoop, SyncLoops())
    SYNC_PROP_MODIFY(space_loop, mSpaceLoop, SyncLoops())
    SYNC_PROP(trans, mTrans)
    SYNC_PROP(about_z, mAboutZ)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS
