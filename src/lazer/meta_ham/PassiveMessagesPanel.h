#pragma once
#include "PassiveMessenger.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"

class PassiveMessagesPanel : public UIPanel {
public:
    virtual ~PassiveMessagesPanel();
    OBJ_CLASSNAME(PassiveMessagesPanel)
    OBJ_SET_TYPE(PassiveMessagesPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();

    PassiveMessagesPanel();

protected:
    PassiveMessenger *unk38; // 0x38
};
