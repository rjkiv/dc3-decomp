#pragma once
#include "obj/Object.h"
#include "ui/UILabel.h"
#include "os/JoypadMsgs.h"
#include "utl/MemMgr.h"

/**
 * @brief A base implementation of a button.
 * Original _objects description:
 * "Simple button, basically just a
 * label that can be selected"
 */
class UIButton : public UILabel {
public:
    OBJ_CLASSNAME(UIButton);
    OBJ_SET_TYPE(UIButton);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);

    OBJ_MEM_OVERLOAD(0x15);
    NEW_OBJ(UIButton)
    REGISTER_OBJ(UIButton)

    static void Init();

private:
    DataNode OnMsg(const ButtonDownMsg &);

protected:
    UIButton();
};
