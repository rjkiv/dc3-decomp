#include "rndobj/TexBlendController.h"
#include "math/Mtx.h"
#include "obj/Object.h"

RndTexBlendController::RndTexBlendController()
    : mMesh(this), mObject1(this), mObject2(this), mReferenceDistance(0), mMinDistance(0),
      mMaxDistance(0), mOverrideMap(this) {}

BEGIN_HANDLERS(RndTexBlendController)
    HANDLE_ACTION(set_min_distance, UpdateMinDistance())
    HANDLE_ACTION(set_max_distance, UpdateMaxDistance())
    HANDLE_ACTION(set_base_distance, UpdateReferenceDistance())
    HANDLE_ACTION(set_all_distances, UpdateAllDistances())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndTexBlendController)
    SYNC_PROP_MODIFY(reference_object_1, mObject1, UpdateAllDistances())
    SYNC_PROP_MODIFY(reference_object_2, mObject2, UpdateAllDistances())
    SYNC_PROP(mesh, mMesh)
    SYNC_PROP(base_distance, mReferenceDistance)
    SYNC_PROP(min_distance, mMinDistance)
    SYNC_PROP(max_distance, mMaxDistance)
    SYNC_PROP(override_map, mOverrideMap)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndTexBlendController)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMesh;
    bs << mObject1;
    bs << mObject2;
    bs << mReferenceDistance;
    bs << mMinDistance;
    bs << mMaxDistance;
    bs << mOverrideMap;
END_SAVES

BEGIN_COPYS(RndTexBlendController)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndTexBlendController)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMesh)
        COPY_MEMBER(mObject1)
        COPY_MEMBER(mObject2)
        COPY_MEMBER(mReferenceDistance)
        COPY_MEMBER(mMinDistance)
        COPY_MEMBER(mMaxDistance)
        COPY_MEMBER(mOverrideMap)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RndTexBlendController)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mMesh;
    d >> mObject1;
    d >> mObject2;
    d >> mReferenceDistance;
    d >> mMinDistance;
    d >> mMaxDistance;
    if (d.rev > 1) {
        d >> mOverrideMap;
    }
END_LOADS

bool RndTexBlendController::IsValid() const {
    if (!mMesh) {
        return false;
    }
    if (!mOverrideMap) {
        if (mMinDistance > mReferenceDistance && mMaxDistance < mReferenceDistance) {
            return mObject1 && mObject2 && mReferenceDistance > 0;
        }
    }
    return true;
}

bool RndTexBlendController::GetCurrentDistance(float &dist) const {
    if (mObject1 && mObject2) {
        const Transform &t = mObject1->WorldXfm();
        dist = Distance(t.v, mObject2->WorldXfm().v);
        return true;
    } else {
        dist = 0;
        return false;
    }
}

void RndTexBlendController::UpdateReferenceDistance() {
    GetCurrentDistance(mReferenceDistance);
    mMinDistance = Min(mMinDistance, mReferenceDistance);
    mMaxDistance = Max(mMaxDistance, mReferenceDistance);
}

void RndTexBlendController::UpdateMinDistance() {
    GetCurrentDistance(mMinDistance);
    mMinDistance = Min(mMinDistance, mReferenceDistance);
}

void RndTexBlendController::UpdateMaxDistance() {
    GetCurrentDistance(mMaxDistance);
    mMaxDistance = Max(mMaxDistance, mReferenceDistance);
}

void RndTexBlendController::UpdateAllDistances() {
    UpdateReferenceDistance();
    mMinDistance = mReferenceDistance * 0.5f;
    mMaxDistance = mReferenceDistance * 1.5f;
}
