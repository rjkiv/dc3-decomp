#pragma once
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "ui/UIListSlot.h"
#include "ui/UILabel.h"
#include "utl/MemMgr.h"

/** "Custom slot for use with UIList" */
class UIListLabel : public UIListSlot {
public:
    OBJ_CLASSNAME(UIListLabel)
    OBJ_SET_TYPE(UIListLabel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual RndTransformable *RootTrans();

    const char *GetDefaultText() const;
    UILabel *ElementLabel(int) const;

    NEW_OBJ(UIListLabel)
    OBJ_MEM_OVERLOAD(0x11)

protected:
    virtual UIListSlotElement *CreateElement(UIList *);
    UIListLabel();

    /** "label to draw/transform" */
    ObjPtr<UILabel> mLabel; // 0x78
    bool unk8c;
};

class UIListLabelElement : public UIListSlotElement {
public:
    UIListLabelElement(UIListLabel *ll, UILabel *l) : mListLabel(ll), mLabel(l) {}
    virtual ~UIListLabelElement();
    virtual void Fill(const UIListProvider &prov, int i, int j) {
        prov.Text(i, j, mListLabel, mLabel);
    }
    virtual void Draw(const Transform &, float, UIColor *, Box *);

    MEM_OVERLOAD(UIListSlotElement, 0x1e) // thats what it says

    UIListLabel *mListLabel;
    UILabel *mLabel;
};
