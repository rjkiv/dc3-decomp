#include "char/CharLookAt.h"
#include "char/CharWeightable.h"
#include "math/Rand.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Graph.h"
#include "rndobj/Poll.h"

const float sMaxThreshold = 80;
bool CharLookAt::sDisableJitter = false;

void DrawBounds(Vector3, const Hmx::Matrix3 &, const Vector3 &, RndGraph *);

CharLookAt::CharLookAt()
    : mSource(this), mPivot(this), mTarget(this), mHalfTime(0), mMinYaw(-80), mMaxYaw(80),
      mMinPitch(-80), mMaxPitch(sMaxThreshold), mMinWeightYaw(-1), mMaxWeightYaw(-1),
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

INIT_REVS(5, 0)

BEGIN_LOADS(CharLookAt)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    d >> mSource;
    d >> mPivot;
    d >> mTarget;
    d >> mHalfTime;
    d >> mMinYaw;
    d >> mMaxYaw;
    d >> mMinPitch;
    d >> mMaxPitch;
    if (d.rev > 1) {
        d >> mMinWeightYaw;
        d >> mMaxWeightYaw;
        d >> mWeightYawSpeed;
    }
    if (d.rev < 3)
        mAllowRoll = true;
    else
        d >> mAllowRoll;
    if (d.rev < 4) {
        mEnableJitter = false;
        mPitchJitterLimit = 0;
        mYawJitterLimit = 0;
    } else {
        d >> mEnableJitter;
        d >> mPitchJitterLimit;
        d >> mYawJitterLimit;
    }
    if (d.rev > 4)
        d >> mSourceRadius;
    SyncLimits();
END_LOADS

void CharLookAt::Highlight() {
    if (mSource && mTarget) {
        RndGraph *graph = RndGraph::GetOneFrame();
        graph->AddLine(
            GetSource()->WorldXfm().v, mTarget->WorldXfm().v, Hmx::Color(1, 0, 0), false
        );
        Hmx::Matrix3 pivotMtx = mPivot->TransParent()->WorldXfm().m;
        Vector3 pivotVec = mPivot->WorldXfm().v;
        DrawBounds(Vector3(mBounds.mMin.x, mBounds.mMin.y, 0), pivotMtx, pivotVec, graph);
        DrawBounds(Vector3(mBounds.mMax.x, mBounds.mMin.y, 0), pivotMtx, pivotVec, graph);
        DrawBounds(Vector3(mBounds.mMin.y, mBounds.mMin.z, 0), pivotMtx, pivotVec, graph);
        DrawBounds(Vector3(mBounds.mMin.y, mBounds.mMax.z, 0), pivotMtx, pivotVec, graph);
    }
}

void CharLookAt::Poll() {
    RndTransformable *srcTrans = GetSource();
    float deltasecs = TheTaskMgr.DeltaSeconds();
    if (mTarget && mPivot) {
        if (!mPivot->TransParent() || !srcTrans || deltasecs < 0)
            return;
        else {
            Vector3 ve4;
            Subtract(mTarget->WorldXfm().v, srcTrans->WorldXfm().v, ve4);
            float charweight = Weight();
            if (mMinWeightYaw >= 0.0f) {
                Vector3 vf0(srcTrans->WorldXfm().m.y);
                Normalize(vf0, vf0);
                Vector3 vfc(ve4);
                vfc.z = 0;
                vf0.z = 0;
                float times = Dot(vf0, vfc);
                float clamped = Clamp(-1.0f, 1.0f, times / (Length(vf0) * Length(vfc)));
                float clamped2 = Clamp<float>(
                    0.0f,
                    1.0f,
                    mMaxWeightYaw - (std::acos(clamped) / (mMaxWeightYaw - mMinWeightYaw))
                );
                float loc13c = (clamped2 - unk9c) / deltasecs;
                if (MinEq(loc13c, mWeightYawSpeed)) {
                    clamped2 = loc13c * deltasecs + unk9c;
                }
                charweight *= clamped2;
                unk9c = clamped2;
            }
            if (charweight != 0.0f) {
                Vector3 v108(0.0f, 0.0f, 0.0f);
                if (mSourceRadius > 0.0f) {
                    if (TheTaskMgr.DeltaSeconds() > 0.0f) {
                        Interp(unka4, srcTrans->WorldXfm().m.y, 0.1f, unka4);
                    }
                    Subtract(srcTrans->WorldXfm().m.y, unka4, v108);
                    float v108sq = LengthSquared(v108);
                    float srcrad = mSourceRadius * DEG2RAD;
                    if (srcrad * srcrad < v108sq) {
                        v108 *= srcrad / std::sqrt(v108sq);
                    }
                }
                if (srcTrans != mPivot) {
                    Transform tf90(mPivot->WorldXfm());
                    Hmx::Quat q118;
                    MakeRotQuat(srcTrans->WorldXfm().m.y, ve4, q118);
                    Hmx::Matrix3 mb4;
                    MakeRotMatrix(q118, mb4);
                    Multiply(tf90.m, mb4, tf90.m);
                    mPivot->SetWorldXfm(tf90);
                    Subtract(mTarget->WorldXfm().v, srcTrans->WorldXfm().v, ve4);
                    MakeRotQuat(srcTrans->WorldXfm().m.y, ve4, q118);
                    MakeRotMatrix(q118, mb4);
                    Multiply(tf90.m.y, mb4, ve4);
                } else
                    Normalize(ve4, ve4);
                Multiply(mPivot->TransParent()->WorldXfm().m, ve4, ve4);
                Normalize(ve4, ve4);
                unke1 = mBounds.Clamp(ve4);
                Normalize(ve4, ve4);
                if (unk8c.x != kHugeFloat && mHalfTime != 0.0f) {
                    Interp(unk8c, ve4, deltasecs / (deltasecs + mHalfTime), ve4);
                }
                unk8c = ve4;
                if (mTestRange) {
                    float loc140, loc144;
                    Interp(mBounds.mMin.z, mBounds.mMax.z, mTestRangeYaw, loc140);
                    Interp(mBounds.mMin.x, mBounds.mMax.x, mTestRangePitch, loc144);
                    ve4.Set(loc144, mBounds.mMin.y, loc140);
                } else if (mShowRange) {
                    charweight = 1.0f;
                    switch (((int)TheTaskMgr.Seconds(TaskMgr::kRealTime)) & 7) {
                    case 0:
                        ve4.Set(mBounds.mMin.x, mBounds.mMin.y, mBounds.mMin.z);
                        break;
                    case 1:
                        ve4.Set(0.0f, mBounds.mMin.z, mBounds.mMax.x);
                        break;
                    case 2:
                        ve4.Set(mBounds.mMax.x, mBounds.mMin.y, mBounds.mMin.z);
                        break;
                    case 3:
                        ve4.Set(mBounds.mMax.x, mBounds.mMin.y, 0.0f);
                        break;
                    case 4:
                        ve4.Set(mBounds.mMax.x, mBounds.mMin.y, mBounds.mMax.z);
                        break;
                    case 5:
                        ve4.Set(0.0f, mBounds.mMin.y, mBounds.mMax.z);
                        break;
                    case 6:
                        ve4.Set(mBounds.mMin.x, mBounds.mMin.y, mBounds.mMax.z);
                        break;
                    case 7:
                        ve4.Set(mBounds.mMin.x, mBounds.mMin.y, 0.0f);
                        break;
                    default:
                        break;
                    }
                }
                static DataNode &disable = DataVariable("cheat.disable_eye_jitter");
                if (mEnableJitter && !sDisableJitter && !disable && deltasecs > 0.0f) {
                    ve4.Set(
                        ve4[0]
                            + RandomFloat(-mPitchJitterLimit, mPitchJitterLimit)
                                * DEG2RAD,
                        ve4[1],
                        ve4[2] + RandomFloat(-mYawJitterLimit, mYawJitterLimit) * DEG2RAD
                    );
                }
                if (mSourceRadius > 0.0f) {
                    Multiply(mPivot->TransParent()->WorldXfm().m, v108, v108);
                    ve4 -= v108;
                }
                if (mAllowRoll) {
                    Hmx::Quat q128;
                    MakeRotQuat(mPivot->LocalXfm().m.y, ve4, q128);
                    FastInterp(Hmx::Quat(0, 0, 0, 1.0f), q128, charweight, q128);
                    Hmx::Matrix3 md8;
                    MakeRotMatrix(q128, md8);
                    if (md8.x.x < -2.0f || md8.x.x > 2.0f) {
                        MILO_NOTIFY_ONCE(
                            "%s has m.x.x %g, character or target scaled or NAN",
                            PathName(this),
                            md8.x.x
                        );
                        md8.Identity();
                    }
                    Multiply(mPivot->LocalXfm().m, md8, mPivot->DirtyLocalXfm().m);
                } else {
                    Hmx::Matrix3 &temp_dirty = mPivot->DirtyLocalXfm().m;
                    Interp(temp_dirty.y, ve4, charweight, temp_dirty.y);
                    temp_dirty.z.Set(-1.0f, 0.0f, 0.0f);
                    Normalize(temp_dirty.y, temp_dirty.y);
                    Cross(temp_dirty.y, temp_dirty.z, temp_dirty.x);
                    Normalize(temp_dirty.x, temp_dirty.x);
                    Cross(temp_dirty.x, temp_dirty.y, temp_dirty.z);
                    if (temp_dirty.x.x < -2.0f || temp_dirty.x.x > 2.0f) {
                        MILO_NOTIFY_ONCE(
                            "%s has m.x.x %g, character or target scaled or NAN",
                            PathName(this),
                            temp_dirty.x.x
                        );
                        temp_dirty.Identity();
                    }
                }
            }
        }
    }
}

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
    changedBy.push_back(GetSource());
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

void CharLookAt::SyncLimits() {
    ClampEq(mMinYaw, -sMaxThreshold, sMaxThreshold);
    ClampEq(mMaxYaw, -sMaxThreshold, sMaxThreshold);
    ClampEq(mMinPitch, -sMaxThreshold, sMaxThreshold);
    ClampEq(mMaxPitch, -sMaxThreshold, sMaxThreshold);
    float yaw = Max<float>(fabsf(mMinYaw), fabsf(mMaxYaw));
    float pitch = Max<float>(fabsf(mMinPitch), fabsf(mMaxPitch));
    mBounds.mMin.y = (float)std::cos(Max<float>(yaw, pitch) * DEG2RAD);
    mBounds.mMax.y = kHugeFloat;
    mBounds.mMin.z = (float)std::tan(mMinYaw * DEG2RAD) * mBounds.mMin.y;
    mBounds.mMax.z = (float)std::tan(mMaxYaw * DEG2RAD) * mBounds.mMin.y;
    mBounds.mMin.x = (float)std::tan(mMinPitch * DEG2RAD) * mBounds.mMin.y;
    mBounds.mMax.x = (float)std::tan(mMaxPitch * DEG2RAD) * mBounds.mMin.y;
}
