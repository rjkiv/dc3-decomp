#include "meta_ham/EventDialogPanel.h"
#include "EventDialogPanel.h"
#include "HamPanel.h"
#include "UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "ui/UIComponent.h"
#include "utl/Symbol.h"

EventDialogPanel::EventDialogPanel() {
    TheUIEventMgr->AddSink(this, EventDialogStartMsg::Type());
    TheUIEventMgr->AddSink(this, EventDialogDismissMsg::Type());
}

EventDialogPanel::~EventDialogPanel() { TheUIEventMgr->RemoveSink(this); }

DataNode EventDialogPanel::OnMsg(EventDialogStartMsg const &msg) {
    MILO_ASSERT(msg.EventType(), 0x23);
    SetTypeDef(msg.EventType());
    HandleType(msg.EventAction());
    mFocusName = gNullStr;
    return 1;
}

DataNode EventDialogPanel::OnMsg(ButtonDownMsg const &msg) {
    if (msg.GetAction() == kAction_Cancel)
        return 0;
    else
        return DataNode(kDataUnhandled, 0);
}

BEGIN_HANDLERS(EventDialogPanel)
    HANDLE_MESSAGE(EventDialogStartMsg)
    HANDLE_MESSAGE(EventDialogDismissMsg)
    HANDLE_MESSAGE(UIComponentSelectDoneMsg)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
