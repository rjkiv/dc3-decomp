#include "meta_ham/EventDialogPanel.h"
#include "EventDialogPanel.h"
#include "HamPanel.h"
#include "UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "ui/UIComponent.h"

EventDialogPanel::EventDialogPanel() {
    TheUIEventMgr->AddSink(this, EventDialogStartMsg::Type());
    TheUIEventMgr->AddSink(this, EventDialogDismissMsg::Type());
}

EventDialogPanel::~EventDialogPanel() { TheUIEventMgr->RemoveSink(this); }

BEGIN_HANDLERS(EventDialogPanel)
    HANDLE_MESSAGE(EventDialogStartMsg)
    HANDLE_MESSAGE(EventDialogDismissMsg)
    HANDLE_MESSAGE(UIComponentSelectDoneMsg)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
