#include "ui/UIColor.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

void UIColor::SetColor(const Hmx::Color &color) { mColor = color; }
UIColor::UIColor() : mColor(1, 1, 1, 1) {}

void UIColor::Save(BinStream &bs) {
    bs << 0;
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mColor;
}

BEGIN_COPYS(UIColor)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(UIColor)
    MILO_ASSERT(c, 0x34);
    COPY_MEMBER(mColor)
END_COPYS

BEGIN_HANDLERS(UIColor)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(UIColor)
    SYNC_PROP(color, mColor)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_LOADS(UIColor)
    LOAD_REVS(bs);
    ASSERT_REVS(0, 0);
    Hmx::Object::Load(bs);
    bs >> mColor;
END_LOADS
