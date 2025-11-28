#include "ui/UIListCustom.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "ui/UIList.h"
#include "ui/UIListSlot.h"

#pragma region UIListCustom

UIListCustom::UIListCustom() : mObject(this) {}

BEGIN_PROPSYNCS(UIListCustom)
    SYNC_PROP_SET(object, mObject.Ptr(), SetObject(_val.GetObj(nullptr)))
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

UIListSlotElement *UIListCustom::CreateElement(UIList *) {
    MILO_ASSERT(mObject, 0x69);
    Hmx::Object *c = Hmx::Object::NewObject(mObject->ClassName());
    c->Copy(mObject.Ptr(), kCopyDeep);
    return new UIListCustomElement(this, c);
}

RndTransformable *UIListCustom::RootTrans() {
    return dynamic_cast<RndTransformable *>(mObject.Ptr());
}

BEGIN_HANDLERS(UIListCustom)
    HANDLE_SUPERCLASS(UIListSlot)
END_HANDLERS

#pragma endregion UIListCustom
#pragma region UIListCustomElement

UIListCustomElement::~UIListCustomElement() { delete mPtr; }

void UIListCustomElement::Draw(const Transform &tf, float f, UIColor *col, Box *box) {
    RndTransformable *t = dynamic_cast<RndTransformable *>(mPtr);
    MILO_ASSERT(t, 34);
    t->SetWorldXfm(tf);
    UIListCustomTemplate *temp = dynamic_cast<UIListCustomTemplate *>(mPtr);
    if (box) {
        if (temp)
            temp->GrowBoundingBox(*box);
    } else {
        if (temp)
            temp->SetAlphaColor(f, col);
        RndDrawable *d = dynamic_cast<RndDrawable *>(mPtr);
        MILO_ASSERT(d, 49);
        d->Draw();
    }
}

#pragma endregion UIListCustomElement
