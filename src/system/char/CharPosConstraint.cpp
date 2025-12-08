#include "char/CharPosConstraint.h"
#include "obj/Object.h"

CharPosConstraint::CharPosConstraint()
    : mSrc(this), mTargets(this), mBox(Vector3(1, 1, 1), Vector3(-1, -1, -1)) {}
CharPosConstraint::~CharPosConstraint() {}

BEGIN_HANDLERS(CharPosConstraint)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharPosConstraint)
    SYNC_PROP(source, mSrc)
    SYNC_PROP(targets, mTargets)
    SYNC_PROP(box, mBox)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharPosConstraint)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mTargets;
    bs << mSrc;
    bs << mBox;
END_SAVES

BEGIN_COPYS(CharPosConstraint)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharPosConstraint)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTargets)
        COPY_MEMBER(mSrc)
        COPY_MEMBER(mBox)
    END_COPYING_MEMBERS
END_COPYS

void CharPosConstraint::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mSrc);
    for (ObjPtrList<RndTransformable>::iterator it = mTargets.begin();
         it != mTargets.end();
         ++it) {
        change.push_back(*it);
        changedBy.push_back(*it);
    }
}

void CharPosConstraint::Load(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    Hmx::Object::Load(bs);
    bs >> mTargets;
    bs >> mSrc;
    if (d.rev > 1) {
        bs >> mBox;
    } else {
        mBox.Set(Vector3(1.0f, 1.0f, 0.0f), Vector3(-1.0f, -1.0f, 1000.0f));
    }
}
