#include "char/CharIKHand.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"

CharIKHand::CharIKHand()
    : mHand(this), mFinger(this), mTargets(this), mOrientation(true), mStretch(true),
      mScalable(false), mMoveElbow(true), mElbowSwing(0), mAlwaysIKElbow(false),
      mPullShoulder(true), unk8c(0), mConstraintWrist(false), mWristRadians(0),
      mElbowCollide(this), mClockwise(false) {}

CharIKHand::~CharIKHand() {}

BEGIN_HANDLERS(CharIKHand)
    HANDLE_ACTION(measure_lengths, MeasureLengths())
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharIKHand::IKTarget)
    SYNC_PROP(target, o.mTarget)
    SYNC_PROP(extent, o.mExtent)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharIKHand)
    SYNC_PROP_SET(hand, mHand.Ptr(), SetHand(_val.Obj<RndTransformable>()))
    SYNC_PROP(finger, mFinger)
    SYNC_PROP(targets, mTargets)
    SYNC_PROP(orientation, mOrientation)
    SYNC_PROP(stretch, mStretch)
    SYNC_PROP(scalable, mScalable)
    SYNC_PROP(move_elbow, mMoveElbow)
    SYNC_PROP(elbow_swing, mElbowSwing)
    SYNC_PROP(always_ik_elbow, mAlwaysIKElbow)
    SYNC_PROP(constraint_wrist, mConstraintWrist)
    SYNC_PROP(wrist_radians, mWristRadians)
    SYNC_PROP(elbow_collide, mElbowCollide)
    SYNC_PROP(clockwise, mClockwise)
    SYNC_PROP(pull_shoulder, mPullShoulder)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const CharIKHand::IKTarget &t) {
    bs << t.mTarget;
    bs << t.mExtent;
    return bs;
}

BEGIN_SAVES(CharIKHand)
    SAVE_REVS(0xD, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mHand;
    bs << mFinger;
    bs << mTargets;
    bs << mOrientation;
    bs << mStretch;
    bs << mScalable;
    bs << mMoveElbow;
    bs << mElbowSwing;
    bs << mAlwaysIKElbow;
    bs << mConstraintWrist;
    bs << mWristRadians;
    bs << mElbowCollide;
    bs << mClockwise;
    bs << mPullShoulder;
END_SAVES

BEGIN_COPYS(CharIKHand)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharIKHand)
    BEGIN_COPYING_MEMBERS
        SetHand(c->mHand);
        COPY_MEMBER(mFinger)
        COPY_MEMBER(mTargets)
        COPY_MEMBER(mOrientation)
        COPY_MEMBER(mStretch)
        COPY_MEMBER(mScalable)
        COPY_MEMBER(mMoveElbow)
        COPY_MEMBER(mElbowSwing)
        COPY_MEMBER(mAlwaysIKElbow)
        COPY_MEMBER(mConstraintWrist)
        COPY_MEMBER(mWristRadians)
        COPY_MEMBER(mTargets)
        COPY_MEMBER(mElbowCollide)
        COPY_MEMBER(mClockwise)
        COPY_MEMBER(mPullShoulder)
    END_COPYING_MEMBERS
END_COPYS

void CharIKHand::SetHand(RndTransformable *t) {
    mHand = t;
    mHandChanged = true;
}
