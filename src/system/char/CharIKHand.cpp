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

BEGIN_LOADS(CharIKHand)
    LOAD_REVS(bs)
    ASSERT_REVS(0xC, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    bs >> mHand;
    if (d.rev > 4)
        bs >> mFinger;
    else
        mFinger = 0;
    if (d.rev < 3) {
        ObjPtr<RndTransformable> tPtr(this, 0);
        bs >> tPtr;
        mTargets.clear();
        mTargets.push_back(IKTarget(ObjPtr<RndTransformable>(tPtr), 0));
    } else if (d.rev < 0xB) {
        ObjPtrList<RndTransformable> tList(this, kObjListNoNull);
        bs >> tList;
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
        bs >> mElbowSwing;
    else
        mElbowSwing = 0.0f;

    if (d.rev > 6)
        d >> mAlwaysIKElbow;
    if (d.rev > 7) {
        d >> mConstraintWrist;
        d >> mWristRadians;
    }
    if (d.rev == 9) {
        String s;
        d >> s;
        bool b;
        d >> b;
    }
    if (d.rev > 0xB) {
        d >> mElbowCollide;
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
        } else {
            v.x = 0;
            v.y = 0;
            v.z = 0;
        }
    }
}

void CharIKHand::MeasureLengths() {
    if (mHand) {
        if (mHand->TransParent()) {
            if (mHand->TransParent()->TransParent()) {
                float len = Length(mHand->LocalXfm().v);
                float parentlen = Length(mHand->TransParent()->LocalXfm().v);
                unk84 = parentlen * 2.0f * len;
                unk88 = (parentlen * parentlen) + len * len;
                if (unk84 != 0.0f)
                    unk84 = 1.0f / unk84;
                unk8c = len + parentlen;
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
        changedBy.push_back(it->mTarget);
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

CharIKHand::IKTarget::IKTarget(Hmx::Object *owner) : mTarget(owner), mExtent(0) {}

CharIKHand::IKTarget::IKTarget(ObjPtr<RndTransformable> t, float e)
    : mTarget(t), mExtent(e) {}

BinStream &operator>>(BinStream &bs, CharIKHand::IKTarget &t) {
    bs >> t.mTarget;
    bs >> t.mExtent;
    return bs;
}

#pragma endregion CharIKHand::IKTarget
