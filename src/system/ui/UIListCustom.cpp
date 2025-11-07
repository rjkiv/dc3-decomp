#include "ui/UIListCustom.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "ui/UIListSlot.h"

UIListCustom::UIListCustom() : mObject(this) {}

BEGIN_PROPSYNCS(UIListCustom)
    SYNC_SUPERCLASS(UIListSlot)
END_PROPSYNCS

BEGIN_SAVES(UIListCustom)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(UIListSlot)
    bs << mObject;
END_SAVES

BEGIN_COPYS(UIListCustom)
    COPY_SUPERCLASS(UIListSlot)
    CREATE_COPY_AS(UIListCustom, c)
    MILO_ASSERT(c, 0x8b);
    COPY_MEMBER(mObject)
END_COPYS

BEGIN_LOADS(UIListCustom)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(UIListSlot)
    bs >> mObject;
END_LOADS

void UIListCustom::SetObject(Hmx::Object *o) {
    if (o) {
        RndTransformable *is_t = dynamic_cast<RndTransformable *>(o);
        RndDrawable *is_d = dynamic_cast<RndDrawable *>(o);
        if (!is_t)
            MILO_NOTIFY("Object is not transformable");
        if (!is_d)
            MILO_NOTIFY("Object is not drawable");
        if (!is_t || !is_d)
            o = nullptr;
    }
    mObject = o;
}

UIListSlotElement *UIListCustom::CreateElement(UIList *) { return nullptr; }

RndTransformable *UIListCustom::RootTrans() {
    return dynamic_cast<RndTransformable *>(mObject.Ptr());
}
