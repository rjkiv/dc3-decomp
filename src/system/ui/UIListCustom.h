#pragma once
#include "obj/Object.h"
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
