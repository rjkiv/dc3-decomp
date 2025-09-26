#include "char/CharNeckTwist.h"
#include "obj/Object.h"

CharNeckTwist::CharNeckTwist() : mTwist(this), mHead(this) {}

BEGIN_HANDLERS(CharNeckTwist)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharNeckTwist)
    SYNC_PROP(head, mHead)
    SYNC_PROP(twist, mTwist)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharNeckTwist)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mHead;
    bs << mTwist;
END_SAVES

BEGIN_COPYS(CharNeckTwist)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharNeckTwist)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mHead)
        COPY_MEMBER(mTwist)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharNeckTwist)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mHead;
    bs >> mTwist;
END_LOADS

void CharNeckTwist::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mHead);
    change.push_back(mTwist);
}
