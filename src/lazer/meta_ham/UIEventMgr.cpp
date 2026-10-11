#include "meta_ham/UIEventMgr.h"
#include "macros.h"
#include "meta_ham/EventDialogPanel.h"
#include "meta_ham/HamUI.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ui/UI.h"
#include "ui/UIScreen.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

UIEventMgr *TheUIEventMgr;

UIEventMgr::BandEvent::BandEvent(UIEventMgr::EventType t, DataArray *a1, DataArray *a2)
    : mEventType(t), mEventDef(a1), mActive(false) {
    if (a2) {
        mEventInit = a2->Clone(true, true, 0);
    }
}

void UIEventMgr::BandEvent::CacheDestination() {
    mEventDef->FindData("destination_screen", mDestination, false);
}

UIEventMgr::UIEventMgr() {}
UIEventMgr::~UIEventMgr() { DeleteAll(mEventQueue); }

BEGIN_HANDLERS(UIEventMgr)
    HANDLE(trigger_event, OnTriggerEvent)
    HANDLE_ACTION(dismiss_event, DismissEvent(gNullStr))
    HANDLE_EXPR(has_active_transition_event, HasActiveTransitionEvent())
    HANDLE_EXPR(has_active_dialog_event, HasActiveDialogEvent())
    HANDLE_EXPR(current_event, CurrentEvent())
    HANDLE_EXPR(is_transition_event_started, IsTransitionEventStarted())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool UIEventMgr::HasActiveTransitionEvent() const {
    if (mEventQueue.empty())
        return false;
    else
        return mEventQueue[0]->mEventType == kTransition;
}

bool UIEventMgr::HasActiveDialogEvent() const {
    if (mEventQueue.empty())
        return false;
    else
        return mEventQueue[0]->mEventType == kDialog;
}

void UIEventMgr::Init() {
    MILO_ASSERT(!TheUIEventMgr, 0x14);
    TheUIEventMgr = new UIEventMgr();
    TheUIEventMgr->SetName("ui_event_mgr", ObjectDir::Main());
    TheUIEventMgr->SetTypeDef(SystemConfig("ui", "ui_event_mgr"));
}

void UIEventMgr::Terminate() { RELEASE(TheUIEventMgr); }

Symbol UIEventMgr::CurrentEvent() const {
    if (!mEventQueue.empty()) {
        return mEventQueue[0]->mEventDef->Sym(0);
    } else {
        return gNullStr;
    }
}

bool UIEventMgr::IsTransitionEventStarted() const {
    MILO_ASSERT(HasActiveTransitionEvent(), 0xDC);
    static Symbol next_screen("next_screen");
    const char *nextScreen = mEventQueue[0]->mEventDef->FindStr(next_screen);
    const char *bottomScreen;
    if (TheUI->BottomScreen()) {
        bottomScreen = TheUI->BottomScreen()->Name();
    } else {
        bottomScreen = "";
    }
    return streq(nextScreen, bottomScreen);
}

bool UIEventMgr::IsTransitionEventFinished() const {
    MILO_ASSERT(HasActiveTransitionEvent(), 0xD1);
    const char *cc = mEventQueue[0]->mDestination.c_str();
    const char *curScreen;
    if (TheUI->CurrentScreen()) {
        curScreen = TheUI->CurrentScreen()->Name();
    } else {
        curScreen = "";
    }
    return streq(cc, curScreen);
}

void UIEventMgr::ActivateFirstEvent() {
    MILO_ASSERT(mEventQueue.size(), 0x89);
    BandEvent *firstEvent = mEventQueue[0];
    MILO_ASSERT(!firstEvent->mActive, 0x8B);
    firstEvent->mActive = true;
    if (firstEvent->mEventType == kTransition) {
        DataArray *initArr = firstEvent->mEventDef->FindArray("init", false);
        if (initArr) {
            initArr->ExecuteScript(1, nullptr, firstEvent->mEventInit, 2);
        }
        firstEvent->CacheDestination();
        static Symbol next_screen("next_screen");
        TheHamUI.GotoEventScreen(
            ObjectDir::Main()->Find<UIScreen>(firstEvent->mEventDef->FindStr(next_screen))
        );
    } else if (firstEvent->mEventType == kDialog) {
        static Message init_msg("init");
        static EventDialogStartMsg msg(firstEvent->mEventDef, init_msg);
        msg[0] = firstEvent->mEventDef;
        if (firstEvent->mEventInit->Size() == 0) {
            firstEvent->mEventInit = init_msg;
        }
        msg[1] = firstEvent->mEventInit;
        Export(msg, false);
    }
}

void UIEventMgr::DismissEvent(Symbol s1) {
    MILO_ASSERT(!mEventQueue.empty(), 0x2D);
    MILO_ASSERT(mEventQueue.front()->mActive, 0x2E);
    Symbol curEvent = CurrentEvent();
    EventType t = mEventQueue.front()->mEventType;
    delete mEventQueue.front();
    mEventQueue.erase(mEventQueue.begin());
    if (t == 0) {
        static EventDialogDismissMsg dismiss_msg(gNullStr, gNullStr);
        dismiss_msg[0] = curEvent;
        dismiss_msg[1] = s1;
        Export(dismiss_msg, false);
    }
    if (mEventQueue.size() > 0 && !mEventQueue.front()->mActive) {
        ActivateFirstEvent();
    }
}

void UIEventMgr::TriggerEvent(Symbol s1, DataArray *a2) {
    if (!TheUI->InTransition()) {
        UIScreen *curScreen = TheUI->CurrentScreen();
        if (TheUI->CurrentScreen()) {
            static Message msg("allow_event", 0);
            msg[0] = s1;
            DataNode handled = curScreen->HandleType(msg);
            if (handled.Type() != kDataUnhandled && handled.Int() == 0) {
                return;
            }
        }
    }

    while (!mEventQueue.empty()) {
        BandEvent *event = mEventQueue.back();
        if (event->mEventType) {
            break;
        }
        if (mEventQueue.size() == 1 && event->mActive) {
            DismissEvent(s1);
        } else {
            RELEASE(event);
            mEventQueue.pop_back();
        }
    }

    static Symbol dialog_events("dialog_events");
    static Symbol transition_events("transition_events");
    DataArray *dialogArray = TypeDef()->FindArray(dialog_events);
    DataArray *eventArr = dialogArray->FindArray(s1, false);
    EventType eventType;
    if (eventArr) {
        eventType = (EventType)0;
    } else {
        eventArr = TypeDef()->FindArray(transition_events, s1);
        eventType = (EventType)1;
    }
    mEventQueue.push_back(new BandEvent(eventType, eventArr, a2));
    if (mEventQueue.size() == 1) {
        ActivateFirstEvent();
    }
}

DataNode UIEventMgr::OnTriggerEvent(DataArray *a) {
    Symbol s = a->Sym(2);
    DataArray *arr = nullptr;
    if (a->Size() > 3) {
        arr = a->Array(3);
    }
    TriggerEvent(s, arr);
    return 1;
}
