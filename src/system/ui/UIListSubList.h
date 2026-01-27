#pragma once
#include "obj/Object.h"
#include "ui/UIListSlot.h"
#include "ui/UIList.h"
#include "utl/MemMgr.h"

/** "Custom slot for use with UIList" */
class UIListSubList : public UIListSlot {
public:
    OBJ_CLASSNAME(UIListSubList)
    OBJ_SET_TYPE(UIListSubList)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual UIList *SubList(int);
    virtual void Draw(
        const UIListWidgetDrawState &,
        const UIListState &,
        const Transform &,
        UIComponent::State,
        Box *,
        DrawCommand
    );

    NEW_OBJ(UIListSubList)
    OBJ_MEM_OVERLOAD(0x11)

    virtual RndTransformable *RootTrans();

    static int sNextFillSelection;

protected:
    virtual UIListSlotElement *CreateElement(UIList *);

    UIListSubList();

    /** "sub list template" */
    ObjPtr<UIList> mList; // 0x78
};

class UIListSubListElement : public UIListSlotElement {
public:
    UIListSubListElement(UIListSubList *sl, UIList *l) : mSlot(sl), mList(l) {}
    virtual ~UIListSubListElement();
    virtual void Fill(const UIListProvider &, int, int);
    virtual void Draw(const Transform &, float, UIColor *, Box *);
    virtual void Poll() { mList->Poll(); }
    virtual UIList *List() { return mList; }

    MEM_OVERLOAD(UIListSlotElement, 0x1e)

    UIListSubList *mSlot;
    UIList *mList;
};
