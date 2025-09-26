#include "char/CharUpperTwist.h"
#include "obj/Object.h"

CharUpperTwist::CharUpperTwist() : mTwist1(this), mTwist2(this), mUpperArm(this) {}
CharUpperTwist::~CharUpperTwist() {}

BEGIN_HANDLERS(CharUpperTwist)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharUpperTwist)
    SYNC_PROP(upper_arm, mUpperArm)
    SYNC_PROP(twist1, mTwist1)
    SYNC_PROP(twist2, mTwist2)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharUpperTwist)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mUpperArm;
    bs << mTwist1;
    bs << mTwist2;
END_SAVES

BEGIN_COPYS(CharUpperTwist)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharUpperTwist)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mUpperArm)
        COPY_MEMBER(mTwist1)
        COPY_MEMBER(mTwist2)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharUpperTwist)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mUpperArm;
    bs >> mTwist1;
    bs >> mTwist2;
END_LOADS

void CharUpperTwist::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mUpperArm);
    change.push_back(mTwist1);
    change.push_back(mTwist2);
}
