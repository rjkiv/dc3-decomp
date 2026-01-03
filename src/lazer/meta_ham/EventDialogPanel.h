#pragma once
#include "meta_ham/OverlayPanel.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "ui/UIComponent.h"

DECLARE_MESSAGE(EventDialogStartMsg, "event_dialog_start")
EventDialogStartMsg(DataArray *a1, DataArray *a2) : Message(Type(), a1, a2) {}
DataArray *EventType() const { return mData->Array(2); }
DataArray *EventAction() const { return mData->Array(3); }
END_MESSAGE

DECLARE_MESSAGE(EventDialogDismissMsg, "event_dialog_dismiss")
EventDialogDismissMsg(Symbol s1, Symbol s2) : Message(Type(), s1, s2) {}
END_MESSAGE

class EventDialogPanel : public OverlayPanel {
public:
    virtual ~EventDialogPanel();
    OBJ_CLASSNAME(EventDialogPanel)
    OBJ_SET_TYPE(EventDialogPanel)
    virtual DataNode Handle(DataArray *, bool);

    NEW_OBJ(EventDialogPanel)

    EventDialogPanel();

protected:
    DataNode OnMsg(EventDialogStartMsg const &);
    DataNode OnMsg(EventDialogDismissMsg const &);
    DataNode OnMsg(UIComponentSelectDoneMsg const &);
    DataNode OnMsg(ButtonDownMsg const &);
};
