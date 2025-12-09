#include "ui/UIGuide.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

UIGuide::UIGuide() : mType(kGuideVertical), mPos(0.5f) {}

UIGuide::~UIGuide() {}

BEGIN_COPYS(UIGuide)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(UIGuide)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mType)
        COPY_MEMBER(mPos)
    END_COPYING_MEMBERS
END_COPYS

void UIGuide::Save(BinStream &bs) {
    bs << 1;
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mType;
    bs << mPos;
}

BEGIN_LOADS(UIGuide)
    LOAD_REVS(bs);
    ASSERT_REVS(1, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> (int &)mType >> mPos;
END_LOADS

BEGIN_PROPSYNCS(UIGuide)
    SYNC_PROP(pos, mPos)
    SYNC_PROP(type, (int &)mType)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_HANDLERS(UIGuide)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
