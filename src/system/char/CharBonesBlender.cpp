#include "char/CharBonesBlender.h"
#include "char/CharBoneDir.h"
#include "obj/Object.h"

void CharBonesBlender::Enter() { CharBones::Enter(); }

void CharBonesBlender::ReallocateInternal() {
    CharBonesAlloc::ReallocateInternal();
    if (mDest)
        mDest->AddBones(mBones);
    CharBones::Enter();
}

BEGIN_SAVES(CharBonesBlender)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mDest;
    bs << mClipType;
END_SAVES

BEGIN_HANDLERS(CharBonesBlender)
    HANDLE_SUPERCLASS(CharPollable)
    HANDLE_SUPERCLASS(CharBonesAlloc)
END_HANDLERS

void CharBonesBlender::SetDest(CharBonesObject *obj) {
    if (obj != mDest) {
        mDest = obj;
        if (mDest)
            mDest->AddBones(mBones);
    }
}

BEGIN_COPYS(CharBonesBlender)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharBonesBlender)
    BEGIN_COPYING_MEMBERS
        SetClipType(c->mClipType);
        SetDest(c->mDest);
    END_COPYING_MEMBERS
END_COPYS

BEGIN_PROPSYNCS(CharBonesBlender)
    SYNC_PROP_SET(dest, mDest.Ptr(), SetDest(_val.Obj<CharBonesObject>()))
    SYNC_PROP_SET(clip_type, mClipType, SetClipType(_val.Sym()))
    SYNC_SUPERCLASS(CharBonesObject)
END_PROPSYNCS

void CharBonesBlender::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mDest);
}

BEGIN_LOADS(CharBonesBlender)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    ObjPtr<CharBonesObject> boneObjPtr(this);
    d >> boneObjPtr;
    Symbol s;
    if (d.rev > 1)
        d >> s;
    SetClipType(s);
    SetDest(boneObjPtr);
END_LOADS

CharBonesBlender::CharBonesBlender() : mDest(this), mClipType("") {}

void CharBonesBlender::SetClipType(Symbol s) {
    if (s != mClipType) {
        mClipType = s;
        ClearBones();
        CharBoneDir::StuffBones(*this, mClipType);
    }
}

void CharBonesBlender::Poll() {
    if (mBones.empty() || !mDest)
        return;
    Blend(*mDest);
    CharBones::Enter();
}
