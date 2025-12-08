#include "char/CharIKHead.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"

#pragma region CharIKHead

CharIKHead::CharIKHead()
    : mPoints(this), mHead(this), mSpine(this), mMouth(this), mTarget(this),
      mHeadFilter(0, 0, 0), mTargetRadius(0.75), mHeadMat(0.5), mOffset(this),
      mOffsetScale(1, 1, 1), mUpdatePoints(1) {}

CharIKHead::~CharIKHead() {}

BEGIN_HANDLERS(CharIKHead)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKHead)
    SYNC_PROP_MODIFY(head, mHead, UpdatePoints(true))
    SYNC_PROP_MODIFY(spine, mSpine, UpdatePoints(true))
    SYNC_PROP(mouth, mMouth)
    SYNC_PROP(target, mTarget)
    SYNC_PROP(target_radius, mTargetRadius)
    SYNC_PROP(head_mat, mHeadMat)
    SYNC_PROP(offset, mOffset)
    SYNC_PROP(offset_scale, mOffsetScale)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_LOADS(CharIKHead)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    bs >> mHead;
    bs >> mSpine;
    bs >> mMouth;
    bs >> mTarget;
    if (d.rev > 1) {
        bs >> mTargetRadius;
        bs >> mHeadMat;
    }
    if (d.rev > 2) {
        bs >> mOffset;
        bs >> mOffsetScale;
    }
    mUpdatePoints = true;
END_LOADS

BEGIN_SAVES(CharIKHead)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mHead;
    bs << mSpine;
    bs << mMouth;
    bs << mTarget;
    bs << mTargetRadius;
    bs << mHeadMat;
    bs << mOffset;
    bs << mOffsetScale;
END_SAVES

BEGIN_COPYS(CharIKHead)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharIKHead)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mHead)
        COPY_MEMBER(mSpine)
        COPY_MEMBER(mMouth)
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mTargetRadius)
        COPY_MEMBER(mHeadMat)
        COPY_MEMBER(mOffset)
        COPY_MEMBER(mOffsetScale)
        mUpdatePoints = true;
    END_COPYING_MEMBERS
END_COPYS

void CharIKHead::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mMouth);
    changedBy.push_back(mHead);
    changedBy.push_back(mTarget);
    if (GenerationCount(mSpine, mHead) != 0) {
        for (RndTransformable *t = mHead; t != 0 && t != mSpine->TransParent();
             t = t->TransParent()) {
            change.push_back(t);
        }
    }
    change.push_back(mOffset);
}

void CharIKHead::UpdatePoints(bool b) {
    if (b || mUpdatePoints) {
        mUpdatePoints = false;
        mPoints.clear();
        int gencnt = GenerationCount(mSpine, mHead);
        if (gencnt != 0) {
            mPoints.resize(gencnt + 1);
            float f1 = 0.0f;
            int i;
            RndTransformable *curtrans = mHead;
            for (i = 0; i < mPoints.size(); i++) {
                Point &pt = mPoints[i];
                pt.unk0 = curtrans;
                pt.unk18 = Length(curtrans->LocalXfm().v);
                curtrans = curtrans->TransParent();
                f1 += pt.unk18;
            }
            mSpineLength = f1;
            float f2 = 1.0f / f1;
            for (int i = 0; i < mPoints.size(); i++) {
                Point &curPt = mPoints[i];
                curPt.unk1c = f1 * f2;
                f1 = f1 - mPoints[i].unk18;
            }
        }
    }
}

void CharIKHead::Highlight() {
#ifdef MILO_DEBUG
    float weight = Weight();
    if (weight == 0 || !mHead || !mTarget || !mSpine)
        return;
    else {
        UtilDrawSphere(mDebugTarget, mTargetRadius, Hmx::Color(0, 1, 0), 0);
        UtilDrawString(MakeString("%.2f", weight), mDebugTarget, Hmx::Color(1, 1, 1));
        for (int i = 1; i < mPoints.size(); i++) {
            TheRnd.DrawLine(
                mPoints[i].unk20, mPoints[i - 1].unk20, Hmx::Color(1, 0, 0), false
            );
            TheRnd.DrawLine(
                mPoints[i].unk14, mPoints[i - 1].unk14, Hmx::Color(0, 1, 0), false
            );
        }
    }
#endif
}

#pragma endregion CharIKHead
#pragma region CharIKHead::Point

CharIKHead::Point::Point(Hmx::Object *owner)
    : unk0(owner), unk14(0, 0, 0), unk18(0), unk1c(0) {}

CharIKHead::Point::Point(CharIKHead::Point const &point)
    : unk0(point.unk0), unk14(point.unk14), unk18(point.unk18), unk1c(point.unk1c) {}

#pragma endregion CharIKHead::Point
