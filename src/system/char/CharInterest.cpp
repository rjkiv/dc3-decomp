#include "char/CharInterest.h"
#include "CharInterest.h"
#include "math/Rot.h"
#include "obj/Object.h"
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

void CharInterest::SyncMaxViewAngle() { unkf4 = std::cos(mMaxViewAngle * DEG2RAD); }
