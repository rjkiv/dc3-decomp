#include "char/CharLookAt.h"
#include "char/CharWeightable.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"

CharLookAt::CharLookAt()
    : mSource(this), mPivot(this), mTarget(this), mHalfTime(0), mMinYaw(-80), mMaxYaw(80),
      mMinPitch(-80), mMaxPitch(80), mMinWeightYaw(-1), mMaxWeightYaw(-1),
      mWeightYawSpeed(10000), unk8c(kHugeFloat, 0, 0), unk9c(1), mSourceRadius(0),
      unka4(0, 0, 0), mShowRange(false), mTestRange(false), mTestRangePitch(0.5),
      mTestRangeYaw(0.5), mAllowRoll(true), unke1(false), mEnableJitter(false),
      mYawJitterLimit(0), mPitchJitterLimit(0) {
    SyncLimits();
}

CharLookAt::~CharLookAt() {}

BEGIN_HANDLERS(CharLookAt)
    HANDLE_SUPERCLASS(CharPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharLookAt)
    SYNC_PROP(source, mSource)
    SYNC_PROP(pivot, mPivot)
    SYNC_PROP(target, mTarget)
    SYNC_PROP(half_time, mHalfTime)
    SYNC_PROP_SET(min_yaw, mMinYaw, SetMinYaw(_val.Float()))
    SYNC_PROP_SET(max_yaw, mMaxYaw, SetMaxYaw(_val.Float()))
    SYNC_PROP_SET(min_pitch, mMinPitch, SetMinPitch(_val.Float()))
    SYNC_PROP_SET(max_pitch, mMaxPitch, SetMaxPitch(_val.Float()))
    SYNC_PROP(min_weight_yaw, mMinWeightYaw)
    SYNC_PROP(max_weight_yaw, mMaxWeightYaw)
    SYNC_PROP(weight_yaw_speed, mWeightYawSpeed)
    SYNC_PROP(allow_roll, mAllowRoll)
    SYNC_PROP(show_range, mShowRange)
    SYNC_PROP(source_radius, mSourceRadius)
    SYNC_PROP(enable_jitter, mEnableJitter)
    SYNC_PROP(yaw_jitter_limit, mYawJitterLimit)
    SYNC_PROP(pitch_jitter_limit, mPitchJitterLimit)
    SYNC_PROP(test_range, mTestRange)
    SYNC_PROP(test_range_pitch, mTestRangePitch)
    SYNC_PROP(test_range_yaw, mTestRangeYaw)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharLookAt)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mSource;
    bs << mPivot;
    bs << mTarget;
    bs << mHalfTime;
    bs << mMinYaw;
    bs << mMaxYaw;
    bs << mMinPitch;
    bs << mMaxPitch;
    bs << mMinWeightYaw;
    bs << mMaxWeightYaw;
    bs << mWeightYawSpeed;
    bs << mAllowRoll;
    bs << mEnableJitter;
    bs << mPitchJitterLimit;
    bs << mYawJitterLimit;
    bs << mSourceRadius;
END_SAVES

BEGIN_COPYS(CharLookAt)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharLookAt)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mSource)
        COPY_MEMBER(mPivot)
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mHalfTime)
        COPY_MEMBER(mMinYaw)
        COPY_MEMBER(mMaxYaw)
        COPY_MEMBER(mMinPitch)
        COPY_MEMBER(mMaxPitch)
        COPY_MEMBER(mMinWeightYaw)
        COPY_MEMBER(mMaxWeightYaw)
        COPY_MEMBER(mWeightYawSpeed)
        COPY_MEMBER(mAllowRoll)
        COPY_MEMBER(mSourceRadius)
        COPY_MEMBER(mEnableJitter)
        COPY_MEMBER(mYawJitterLimit)
        COPY_MEMBER(mPitchJitterLimit)
    END_COPYING_MEMBERS
    SyncLimits();
END_COPYS

void CharLookAt::Enter() {
    unk8c.Set(kHugeFloat, 0, 0);
    if (mPivot) {
        mPivot->DirtyLocalXfm().m.Identity();
    }
    RndPollable::Enter();
}

void CharLookAt::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mSource);
    changedBy.push_back(mTarget);
    change.push_back(mPivot);
}

void CharLookAt::SetMinYaw(float yaw) {
    mMinYaw = yaw;
    SyncLimits();
}

void CharLookAt::SetMaxYaw(float yaw) {
    mMaxYaw = yaw;
    SyncLimits();
}

void CharLookAt::SetMinPitch(float pitch) {
    mMinPitch = pitch;
    SyncLimits();
}

void CharLookAt::SetMaxPitch(float pitch) {
    mMaxPitch = pitch;
    SyncLimits();
}
