#include "char/CharInterest.h"
#include "CharInterest.h"
#include "math/Rand.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Graph.h"
#include "rndobj/Trans.h"

CharInterest::CharInterest()
    : mMaxViewAngle(20), mPriority(1), mMinLookTime(1), mMaxLookTime(3),
      mRefractoryPeriod(6.1), mDartRulesetOverride(this), mCategoryFlags(0),
      mOverridesMinTargetDist(0), mMinTargetDistOverride(35) {
    SyncMaxViewAngle();
}

BEGIN_HANDLERS(CharInterest)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_LOADS(CharInterest)
    LOAD_REVS(bs)
    ASSERT_REVS(6, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    bs >> mMaxViewAngle;
    bs >> mPriority;
    bs >> mMinLookTime;
    bs >> mMaxLookTime;
    bs >> mRefractoryPeriod;
    if (d.rev > 1 && d.rev <= 5) {
        ObjPtr<Hmx::Object> obj(this);
        bs >> obj;
    } else if (d.rev > 5) {
        bs >> mDartRulesetOverride;
    }
    if (d.rev > 2) {
        bs >> mCategoryFlags;
        if (d.rev == 3) {
            bool x;
            d >> x;
        }
    }
    if (d.rev > 4) {
        d >> mOverridesMinTargetDist;
        bs >> mMinTargetDistOverride;
    }
    SyncMaxViewAngle();
END_LOADS

BEGIN_PROPSYNCS(CharInterest)
    SYNC_PROP_MODIFY(max_view_angle, mMaxViewAngle, SyncMaxViewAngle())
    SYNC_PROP(priority, mPriority)
    SYNC_PROP(min_look_time, mMinLookTime)
    SYNC_PROP(max_look_time, mMaxLookTime)
    SYNC_PROP(refractory_period, mRefractoryPeriod)
    SYNC_PROP(dart_ruleset_override, mDartRulesetOverride)
    SYNC_PROP_BITFIELD(category_flags, mCategoryFlags, 0x138)
    SYNC_PROP(overrides_min_target_dist, mOverridesMinTargetDist)
    SYNC_PROP(min_target_dist_override, mMinTargetDistOverride)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharInterest)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mMaxViewAngle;
    bs << mPriority;
    bs << mMinLookTime;
    bs << mMaxLookTime;
    bs << mRefractoryPeriod;
    bs << mDartRulesetOverride;
    bs << mCategoryFlags;
    bs << mOverridesMinTargetDist;
    bs << mMinTargetDistOverride;
END_SAVES

BEGIN_COPYS(CharInterest)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(CharInterest)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMaxViewAngle)
        COPY_MEMBER(mPriority)
        COPY_MEMBER(mMinLookTime)
        COPY_MEMBER(mMaxLookTime)
        COPY_MEMBER(mRefractoryPeriod)
        COPY_MEMBER(mDartRulesetOverride)
        COPY_MEMBER(mCategoryFlags)
        COPY_MEMBER(mOverridesMinTargetDist)
        COPY_MEMBER(mMinTargetDistOverride)
        SyncMaxViewAngle();
    END_COPYING_MEMBERS
END_COPYS

void CharInterest::SyncMaxViewAngle() {
    mMaxViewAngleCos = std::cos(mMaxViewAngle * DEG2RAD);
}

void CharInterest::Highlight() {
    RndGraph *oneframe = RndGraph::GetOneFrame();
    oneframe->AddSphere(WorldXfm().v, 1.0f, Hmx::Color(1.0f, 0.0f, 0.0f));
    Vector2 vec2;
    float wts = RndCam::Current()->WorldToScreen(WorldXfm().v, vec2);
    if (wts > 0.0f) {
        vec2.x = vec2.x * TheRnd.Width();
        vec2.y = vec2.y * TheRnd.Height();
        vec2.y += 15.0;
        vec2.x -= 30.0;
        oneframe->AddString(MakeString("%s", Name()), vec2, Hmx::Color(1.0f, 1.0f, 1.0f));
    }
    if (mDartRulesetOverride) {
        const DataNode *minrad = mDartRulesetOverride->Property("min_radius", false);
        const DataNode *maxrad = mDartRulesetOverride->Property("max_radius", false);
        if (minrad && maxrad) {
            oneframe->AddSphere(
                WorldXfm().v, minrad->Float(), Hmx::Color(0.7f, 0.7f, 0.7f)
            );
            oneframe->AddSphere(
                WorldXfm().v, maxrad->Float(), Hmx::Color(1.0f, 1.0f, 1.0f)
            );
        }
    }
}

bool CharInterest::IsWithinViewCone(const Vector3 &v1, const Vector3 &v2) {
    Vector3 v1c;
    v1c = WorldXfm().v;
    Vector3 v28;
    Subtract(v1c, v1, v28);
    Normalize(v28, v28);
    if (Dot(v2, v28) >= mMaxViewAngleCos)
        return true;
    else
        return false;
}

bool CharInterest::IsMatchingFilterFlags(int mask) {
    return (mCategoryFlags & mask) == mCategoryFlags && mCategoryFlags != 0;
}

float CharInterest::ComputeScore(
    const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, float f, int filterFlags, bool b
) {
    bool b2 = false;
    if (IsMatchingFilterFlags(filterFlags) || (b && mCategoryFlags == 0)) {
        b2 = true;
    }
    if (!b2)
        return -1.0f;

    Vector3 v7c(WorldXfm().v);
    Vector3 v88;
    Subtract(v7c, v2, v88);
    float lensq = LengthSquared(v88);
    Normalize(v88, v88);

    float dot = Dot(v1, v88);
    float f1 = 0.0f;
    if (dot >= mMaxViewAngleCos)
        f1 = 1.0f;

    float dot2 = Dot(v3, v88);
    float f2 = 0.0f;
    if (dot2 >= mMaxViewAngleCos)
        f2 = 1.0f;

    float f7 = -(lensq * f - 1.0f);
    if (IsNaN(f7)) {
        MILO_NOTIFY("error scoring interest object: bad normalize factor gave score %f", f7);
    }

    float f4 = f7 + f2 + f1 - 0.99f;
    if (f4 >= 0.0f) {
        f4 = f4 + RandomFloat(-0.25f, 0.25f);
    }
    return f4 * mPriority;
}
