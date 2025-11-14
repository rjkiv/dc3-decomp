#include "char/CharTransDraw.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"

CharTransDraw::CharTransDraw() : mChars(this), unk54(false) {}

CharTransDraw::~CharTransDraw() {}

BEGIN_PROPSYNCS(CharTransDraw)
    SYNC_PROP(chars, mChars)
    SYNC_PROP(force_draw, unk54)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharTransDraw)
    SAVE_REVS(2, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mChars;
    bs << unk54;
END_SAVES

BEGIN_COPYS(CharTransDraw)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(CharTransDraw)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mChars)
        COPY_MEMBER(unk54)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharTransDraw)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 1)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    bs >> mChars;
    if (d.altRev > 0)
        bs >> unk54;
END_LOADS

void CharTransDraw::DrawShowing() {}

BEGIN_HANDLERS(CharTransDraw)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
