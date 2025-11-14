#include "char/CharBlendBone.h"
#include "obj/Object.h"
#include "os/Debug.h"

#pragma region CharBlendBone

CharBlendBone::CharBlendBone()
    : mTargets(this), mSrc1(this), mSrc2(this), mTransX(false), mTransY(false),
      mTransZ(false), mRotation(false), mSetLocal(false) {}

BEGIN_PROPSYNCS(CharBlendBone)
    SYNC_PROP(target, mTargets)
    SYNC_PROP(src_one, mSrc1)
    SYNC_PROP(src_two, mSrc2)
    SYNC_PROP(trans_x, mTransX)
    SYNC_PROP(trans_y, mTransY)
    SYNC_PROP(trans_z, mTransZ)
    SYNC_PROP(rotation, mRotation)
    SYNC_PROP(set_local, mSetLocal)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharBlendBone)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs >> mTargets;
    bs << mSrc1;
    bs << mSrc2;
    bs << mTransX;
    bs << mTransY;
    bs << mTransZ;
    bs << mRotation;
    bs << mSetLocal;
END_SAVES

BEGIN_COPYS(CharBlendBone)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharBlendBone)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTargets)
        COPY_MEMBER(mSrc1)
        COPY_MEMBER(mSrc2)
        COPY_MEMBER(mTransX)
        COPY_MEMBER(mTransY)
        COPY_MEMBER(mTransZ)
        COPY_MEMBER(mRotation)
        COPY_MEMBER(mSetLocal)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharBlendBone)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    MILO_ASSERT(d.rev > 2, 0x66);
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mTargets;
    bs >> mSrc1;
    bs >> mSrc2;
    d >> mTransX;
    d >> mTransY;
    d >> mTransZ;
    d >> mRotation;
    if (3 < d.rev) {
        d >> mSetLocal;
    }
END_LOADS

void CharBlendBone::Poll() {}

void CharBlendBone::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mSrc1);
    changedBy.push_back(mSrc2);
    for (ObjVector<ConstraintSystem>::iterator it = mTargets.begin();
         it != mTargets.end();
         ++it) {
        change.push_back((*it).mTarget);
    }
}

BEGIN_HANDLERS(CharBlendBone)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion CharBlendBone
#pragma region CharBlendBone::ConstraintSystem

CharBlendBone::ConstraintSystem::ConstraintSystem(Hmx::Object *o)
    : mTarget(o), mWeight(0.5f) {}

BinStream &operator>>(BinStream &bs, CharBlendBone::ConstraintSystem &cs) {
    bs >> cs.mTarget;
    bs >> cs.mWeight;
    return bs;
}

BEGIN_CUSTOM_PROPSYNC(CharBlendBone::ConstraintSystem)
    SYNC_PROP(target, o.mTarget)
    SYNC_PROP(weight, o.mWeight)
END_CUSTOM_PROPSYNC

#pragma endregion CharBlendBone::ConstraintSystem
