#include "char/CharBoneOffset.h"
#include "char/CharPollable.h"
#include "obj/Object.h"

CharBoneOffset::CharBoneOffset() : mDest(this), mOffset(0, 0, 0) {}

BEGIN_HANDLERS(CharBoneOffset)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharBoneOffset)
    SYNC_PROP(dest, mDest)
    SYNC_PROP(offset, mOffset)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharBoneOffset)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mDest;
    bs << mOffset;
END_SAVES

BEGIN_COPYS(CharBoneOffset)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharBoneOffset)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDest)
        COPY_MEMBER(mOffset)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharBoneOffset)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mDest;
    d >> mOffset;
END_LOADS

void CharBoneOffset::Poll() {
    if (!mDest || !mDest->TransParent())
        return;
    Transform tf(mDest->LocalXfm());
    tf.v += mOffset;
    Transform tRes;
    Multiply(tf, mDest->TransParent()->WorldXfm(), tRes);
    mDest->SetWorldXfm(tRes);
}

void CharBoneOffset::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mDest);
    if (mDest && mDest->TransParent())
        changedBy.push_back(mDest->TransParent());
}
