#include "char/CharMeshHide.h"
#include "obj/Object.h"

#pragma region CharMeshHide::Hide

CharMeshHide::Hide::Hide(Hmx::Object *o) : mDraw(o), mFlags(0), mShow(0) {}

CharMeshHide::Hide::Hide(const CharMeshHide::Hide &hide)
    : mDraw(hide.mDraw), mFlags(hide.mFlags), mShow(hide.mShow) {}

CharMeshHide::Hide &CharMeshHide::Hide::operator=(const CharMeshHide::Hide &hide) {
    mDraw = hide.mDraw;
    mFlags = hide.mFlags;
    mShow = hide.mShow;
    return *this;
}

BEGIN_CUSTOM_PROPSYNC(CharMeshHide::Hide)
    SYNC_PROP(drawable, o.mDraw)
    SYNC_PROP(flags, o.mFlags)
    SYNC_PROP(show, o.mShow)
END_CUSTOM_PROPSYNC

BinStream &operator>>(BinStream &bs, CharMeshHide::Hide &hide) {
    bs >> hide.mDraw;
    bs >> hide.mFlags;
    bs >> hide.mShow;
    return bs;
}

#pragma endregion CharMeshHide::Hide
#pragma region CharMeshHide

CharMeshHide::CharMeshHide() : mHides(this), mFlags(0) {}

CharMeshHide::~CharMeshHide() {}

BEGIN_PROPSYNCS(CharMeshHide)
    SYNC_PROP(flags, mFlags)
    SYNC_PROP(hides, mHides)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharMeshHide)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mFlags;
END_SAVES

BEGIN_COPYS(CharMeshHide)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharMeshHide)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFlags)
        COPY_MEMBER(mHides)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharMeshHide)
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mFlags >> mHides;
END_LOADS

void CharMeshHide::Init() { REGISTER_OBJ_FACTORY(CharMeshHide) }

#pragma endregion CharMeshHide
