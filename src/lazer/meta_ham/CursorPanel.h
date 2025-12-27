#pragma once
#include "PassiveMessagesPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"

class CursorPanel : public PassiveMessagesPanel {
public:
    virtual ~CursorPanel();
    OBJ_CLASSNAME(CursorPanel)
    OBJ_SET_TYPE(CursorPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();

    NEW_OBJ(CursorPanel)

    CursorPanel();
};
