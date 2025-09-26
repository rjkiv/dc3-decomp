#include "char/CharBoneTwist.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"

CharBoneTwist::CharBoneTwist() : mBone(this), mTargets(this) {}

BEGIN_HANDLERS(CharBoneTwist)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharBoneTwist)
    SYNC_PROP(bone, mBone)
    SYNC_PROP(targets, mTargets)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharBoneTwist)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mBone;
    bs << mTargets;
END_SAVES

BEGIN_COPYS(CharBoneTwist)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharBoneTwist)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBone)
        COPY_MEMBER(mTargets)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharBoneTwist)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    bs >> mBone;
    bs >> mTargets;
END_LOADS
