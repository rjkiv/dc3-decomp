#include "char/CharIKHead.h"
#include "char/CharWeightable.h"
#include "char/Character.h"
#include "math/Mtx.h"
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

INIT_REVS(3, 0)

BEGIN_LOADS(CharIKHead)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    d >> mHead;
    d >> mSpine;
    d >> mMouth;
    d >> mTarget;
    if (d.rev > 1) {
        d >> mTargetRadius;
        d >> mHeadMat;
    }
    if (d.rev > 2) {
        d >> mOffset;
        d >> mOffsetScale;
    }
    mUpdatePoints = true;
END_LOADS

void CharIKHead::Highlight() {
    float weight = Weight();
    if (weight && mHead && mTarget && mSpine) {
        UtilDrawSphere(mDebugTarget, mTargetRadius, Hmx::Color(0, 1, 0), 0);
        UtilDrawString(MakeString("%.2f", weight), mDebugTarget, Hmx::Color(1, 1, 1));
        for (int i = 1; i < mPoints.size(); i++) {
            TheRnd.DrawLine(
                mPoints[i].debugPos, mPoints[i - 1].debugPos, Hmx::Color(1, 0, 0), false
            );
            TheRnd.DrawLine(
                mPoints[i].pos, mPoints[i - 1].pos, Hmx::Color(0, 1, 0), false
            );
        }
    }
}

void CharIKHead::Poll() {
    if (mHead && mTarget && mSpine) {
        UpdatePoints(false);
        Character *cur = Character::Current();
        if (cur && cur->Teleported()) {
            mHeadFilter = mHead->WorldXfm().v;
        } else if (TheTaskMgr.DeltaSeconds() > 0) {
            Interp(mHeadFilter, mHead->WorldXfm().v, 0.5f, mHeadFilter);
        }
        float weight = Weight();
        if (weight != 0) {
            Hmx::Matrix3 m58 = mHead->WorldXfm().m;
            Vector3 vec;
            Subtract(mHead->WorldXfm().v, mHeadFilter, vec);
            Vector3 vf8 = mTarget->WorldXfm().v;
            float lenec = Length(vec);
            if (lenec > 0) {
                float f10 = Min(lenec, mTargetRadius * weight);
                ScaleAddEq(vf8, vec, f10 / lenec);
            }
            mDebugTarget = vf8;
            Interp(mPoints[0].trans->WorldXfm().v, vf8, weight, vf8);
            Vector3 v104;
            Subtract(vf8, mSpine->TransParent()->WorldXfm().v, v104);
            float lensq = LengthSquared(v104);
            if (lensq > mSpineLength * mSpineLength) {
                float len = std::sqrt(lensq);
                ScaleAdd(
                    mSpine->TransParent()->WorldXfm().v, v104, mSpineLength / len, vf8
                );
            }
            Subtract(vf8, mPoints[0].trans->WorldXfm().v, v104);
            for (int i = 0; i < mPoints.size(); i++) {
                Point &cur = mPoints[i];
                ScaleAdd(cur.trans->WorldXfm().v, v104, cur.weight, cur.pos);
                cur.debugPos = cur.pos;
            }
            Vector3 v110(0, 0, 0);
            for (int i = 1; i < mPoints.size(); i++) {
                mPoints[i].pos += v110;
                Vector3 v11c;
                Subtract(mPoints[i].pos, mPoints[i - 1].pos, v11c);
                v110 -= v11c;
                NormalizeScale(v11c, mPoints[i - 1].dist, v11c);
                v110 += v11c;
                Add(mPoints[i - 1].pos, v11c, mPoints[i].pos);
            }
            for (int i = 1; i < mPoints.size(); i++) {
                ScaleAddEq(mPoints[i].pos, v110, mPoints[i].weight - 1.0f);
            }
            for (int i = mPoints.size() - 1; i >= 0; i--) {
                Transform tf88(mPoints[i].trans->WorldXfm().m, mPoints[i].pos);
                if (i > 0) {
                    Vector3 v128;
                    Hmx::Quat q138;
                    Multiply(mPoints[i - 1].trans->LocalXfm().v, tf88.m, v128);
                    Vector3 v148;
                    Subtract(mPoints[i - 1].pos, mPoints[i].pos, v148);
                    MakeRotQuat(v128, v148, q138);
                    Hmx::Matrix3 mb0;
                    MakeRotMatrix(q138, mb0);
                    Multiply(tf88.m, mb0, tf88.m);
                } else {
                    Interp(tf88.m, m58, mHeadMat, tf88.m);
                }
                mPoints[i].trans->SetWorldXfm(tf88);
            }
            if (mOffset) {
                Transform tfe0(mOffset->WorldXfm());
                Scale(v104, mOffsetScale, v104);
                tfe0.v += v104;
                mOffset->SetWorldXfm(tfe0);
            }
        }
    }
}

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
                pt.trans = curtrans;
                pt.dist = Length(curtrans->LocalXfm().v);
                curtrans = curtrans->TransParent();
                f1 += pt.dist;
            }
            mSpineLength = f1;
            float f2 = 1.0f / f1;
            for (int i = 0; i < mPoints.size(); i++) {
                Point &curPt = mPoints[i];
                curPt.weight = f1 * f2;
                f1 = f1 - mPoints[i].dist;
            }
        }
    }
}

#pragma endregion CharIKHead
#pragma region CharIKHead::Point

CharIKHead::Point::Point(Hmx::Object *owner)
    : trans(owner), pos(0, 0, 0), dist(0), weight(0) {}

CharIKHead::Point::Point(CharIKHead::Point const &point)
    : trans(point.trans), pos(point.pos), dist(point.dist), weight(point.weight) {}

#pragma endregion CharIKHead::Point
