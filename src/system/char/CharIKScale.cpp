#include "char/CharIKScale.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"

CharIKScale::CharIKScale()
    : mDest(this), mScale(1), mSecondaryTargets(this), mBottomHeight(0), mTopHeight(0),
      mAutoWeight(0) {}

CharIKScale::~CharIKScale() {}

BEGIN_HANDLERS(CharIKScale)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_ACTION(capture_before, CaptureBefore())
    HANDLE_ACTION(capture_after, CaptureAfter())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKScale)
    SYNC_PROP(dest, mDest)
    SYNC_PROP(scale, mScale)
    SYNC_PROP(secondary_targets, mSecondaryTargets)
    SYNC_PROP(auto_weight, mAutoWeight)
    SYNC_PROP(bottom_height, mBottomHeight)
    SYNC_PROP(top_height, mTopHeight)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharIKScale)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mDest;
    bs << mScale;
    bs << mSecondaryTargets;
    bs << mAutoWeight;
    bs << mBottomHeight;
    bs << mTopHeight;
END_SAVES

BEGIN_COPYS(CharIKScale)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharIKScale)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDest)
        COPY_MEMBER(mScale)
        COPY_MEMBER(mSecondaryTargets)
        COPY_MEMBER(mAutoWeight)
        COPY_MEMBER(mBottomHeight)
        COPY_MEMBER(mTopHeight)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharIKScale)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    bs >> mDest;
    bs >> mScale;
    if (d.rev > 1)
        bs >> mSecondaryTargets;
    if (d.rev > 2) {
        bs >> mAutoWeight;
        bs >> mBottomHeight >> mTopHeight;
    }
END_LOADS

void CharIKScale::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mDest);
    for (ObjPtrList<RndTransformable>::iterator it = mSecondaryTargets.begin();
         it != mSecondaryTargets.end();
         ++it) {
        change.push_back(*it);
    }
    changedBy.push_back(mDest);
}

void CharIKScale::CaptureBefore() {
    if (mDest)
        mScale = mDest->LocalXfm().v.z;
}

void CharIKScale::CaptureAfter() {
    if (mDest)
        mScale = mDest->LocalXfm().v.z / mScale;
}
