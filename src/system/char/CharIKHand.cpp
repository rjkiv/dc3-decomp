#include "char/CharIKHand.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "rndobj/Utl.h"

#pragma region CharIKHand

CharIKHand::CharIKHand()
    : mHand(this), mFinger(this), mTargets(this), mOrientation(true), mStretch(true),
      mScalable(false), mMoveElbow(true), mElbowSwing(0), mAlwaysIK(false),
      mPullShoulder(true), mArmLength(0), mConstrainWrist(false), mWristRadians(0),
      mElbowCollisionObj(this), mClockwise(false) {}

CharIKHand::~CharIKHand() {}

BEGIN_HANDLERS(CharIKHand)
    HANDLE_ACTION(measure_lengths, MeasureLengths())
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharIKHand::IKTarget)
    SYNC_PROP(target, o.targetTrans)
    SYNC_PROP(extent, o.extent)
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
    SYNC_PROP(always_ik_elbow, mAlwaysIK)
    SYNC_PROP(constrain_wrist, mConstrainWrist)
    SYNC_PROP(wrist_radians, mWristRadians)
    SYNC_PROP(elbow_collide, mElbowCollisionObj)
    SYNC_PROP(clockwise, mClockwise)
    SYNC_PROP(pull_shoulder, mPullShoulder)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const CharIKHand::IKTarget &t) {
    bs << t.targetTrans;
    bs << t.extent;
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
    bs << mAlwaysIK;
    bs << mConstrainWrist;
    bs << mWristRadians;
    bs << mElbowCollisionObj;
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
        COPY_MEMBER(mAlwaysIK)
        COPY_MEMBER(mConstrainWrist)
        COPY_MEMBER(mWristRadians)
        COPY_MEMBER(mTargets)
        COPY_MEMBER(mElbowCollisionObj)
        COPY_MEMBER(mClockwise)
        COPY_MEMBER(mPullShoulder)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0xC, 0)

BEGIN_LOADS(CharIKHand)
    LOAD_REVS(bs)
    ASSERT_REVS(0xD, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    d >> mHand;
    if (d.rev > 4)
        d >> mFinger;
    else
        mFinger = 0;
    if (d.rev < 3) {
        ObjPtr<RndTransformable> tPtr(this, 0);
        d >> tPtr;
        mTargets.clear();
        mTargets.push_back(IKTarget(ObjPtr<RndTransformable>(tPtr), 0));
    } else if (d.rev < 0xB) {
        ObjPtrList<RndTransformable> tList(this, kObjListNoNull);
        d >> tList;
        mTargets.clear();
        for (ObjPtrList<RndTransformable>::iterator it = tList.begin(); it != tList.end();
             ++it) {
            ObjPtr<RndTransformable> tPtr(this, *it);
            mTargets.push_back(IKTarget(ObjPtr<RndTransformable>(tPtr), 0));
        }
    } else
        d >> mTargets;

    d >> mOrientation;
    d >> mStretch;
    if (d.rev > 1)
        d >> mScalable;
    else
        mScalable = false;

    if (d.rev > 3)
        d >> mMoveElbow;
    else
        mMoveElbow = true;

    if (d.rev > 5)
        d >> mElbowSwing;
    else
        mElbowSwing = 0.0f;

    if (d.rev > 6)
        d >> mAlwaysIK;
    if (d.rev > 7) {
        d >> mConstrainWrist;
        d >> mWristRadians;
    }
    if (d.rev == 9) {
        String s;
        d >> s;
        bool b;
        d >> b;
    }
    if (d.rev > 0xB) {
        d >> mElbowCollisionObj;
        d >> mClockwise;
    }
    if (d.rev > 0xc)
        d >> mPullShoulder;
    SetHand(mHand);
END_LOADS

void CharIKHand::SetHand(RndTransformable *t) {
    mHand = t;
    mHandChanged = true;
}

void CharIKHand::PullShoulder(
    Vector3 &v, const Transform &tf, const Vector3 &vconst, float fff
) {
    if (mPullShoulder) {
        Subtract(vconst, tf.v, v);
        float lensq = LengthSquared(v);
        float f2 = fff * 0.95f;
        if (lensq > f2 * f2) {
            v *= 1.0f - f2 / (float)std::sqrt(lensq);
            return;
        }
    }
    v.Set(0, 0, 0);
}

void CharIKHand::MeasureLengths() {
    if (mHand) {
        if (mHand->TransParent()) {
            if (mHand->TransParent()->TransParent()) {
                float len = Length(mHand->LocalXfm().v);
                float parentlen = Length(mHand->TransParent()->LocalXfm().v);
                mInv2ab = parentlen * 2.0f * len;
                mAAPlusBB = (parentlen * parentlen) + len * len;
                if (mInv2ab != 0.0f)
                    mInv2ab = 1.0f / mInv2ab;
                mArmLength = len + parentlen;
            }
        }
    }
}

void CharIKHand::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mHand);
    changedBy.push_back(mHand);
    change.push_back(mFinger);
    changedBy.push_back(mFinger);
    for (ObjVector<IKTarget>::iterator it = mTargets.begin(); it != mTargets.end();
         ++it) {
        changedBy.push_back(it->targetTrans);
    }
    if (mMoveElbow && mHand) {
        RndTransformable *handParent = mHand->TransParent();
        if (handParent) {
            change.push_back(handParent);
            changedBy.push_back(handParent);
            handParent = handParent->TransParent();
            if (handParent) {
                change.push_back(handParent);
                changedBy.push_back(handParent);
            }
        }
    }
}

#pragma endregion CharIKHand
#pragma region CharIKHand::IKTarget

CharIKHand::IKTarget::IKTarget(Hmx::Object *owner) : targetTrans(owner), extent(0) {}

CharIKHand::IKTarget::IKTarget(ObjPtr<RndTransformable> t, float e)
    : targetTrans(t), extent(e) {}

BinStream &operator>>(BinStream &bs, CharIKHand::IKTarget &t) {
    bs >> t.targetTrans;
    bs >> t.extent;
    return bs;
}

#pragma endregion CharIKHand::IKTarget
