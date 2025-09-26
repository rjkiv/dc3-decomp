#pragma once
#include "math/Key.h"
#include "obj/Msg.h"
#include "ui/UIComponent.h"
#include "ui/UILabel.h"
#include "ui/UITransitionHandler.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class HamMove;

/** "Label with Hammer-specific features" */
class HamLabel : public UILabel, public UITransitionHandler {
public:
    // Hmx::Object
    virtual ~HamLabel();
    OBJ_CLASSNAME(HamLabel);
    OBJ_SET_TYPE(HamLabel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // UILabel
    virtual void Count(int, int, float, Symbol);
    virtual void FinishCount();
    virtual void SetMoveName(HamMove *);
    virtual bool CanHaveFocus() { return mCanHaveFocus; }
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x20)
    NEW_OBJ(HamLabel)
    static void Init();

protected:
    HamLabel();

    virtual void SetDisplayText(const char *, bool);
    // UITransitionHandler
    virtual void FinishValueChange();
    virtual bool IsEmptyValue() const { return mText == gNullStr; }

    Keys<float, float> unk168; // 0x168
    Symbol unk174; // 0x174
    String unk178; // 0x178
    bool unk180; // 0x180
    bool mCanHaveFocus; // 0x181
};

DECLARE_MESSAGE(HamLabelCountDoneMsg, "count_done")
HamLabelCountDoneMsg(UIComponent *c) : Message(Type(), c) {}
END_MESSAGE
