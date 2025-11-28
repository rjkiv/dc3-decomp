#pragma once
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "ui/UIListSlot.h"

/** "Custom slot for use with UIList" */
class UIListCustom : public UIListSlot {
public:
    OBJ_CLASSNAME(UIListCustom)
    OBJ_SET_TYPE(UIListCustom)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);

    void SetObject(Hmx::Object *);

protected:
    UIListCustom();

    virtual UIListSlotElement *CreateElement(UIList *);
    virtual RndTransformable *RootTrans();

    /** "custom object to draw/transform" */
    ObjPtr<Hmx::Object> mObject;
};

class UIListCustomElement : public UIListSlotElement {
public:
    UIListCustomElement(class UIListCustom *own, Hmx::Object *ptr)
        : mOwner(own), mPtr(ptr) {}
    virtual ~UIListCustomElement();
    virtual void Fill(const UIListProvider &p, int i1, int i2) {
        p.Custom(i1, i2, mOwner, mPtr);
    }
    virtual void Draw(const Transform &, float, UIColor *, Box *);

    MEM_OVERLOAD(UIListSlotElement, 0x1e);

    class UIListCustom *mOwner;
    Hmx::Object *mPtr;
};
