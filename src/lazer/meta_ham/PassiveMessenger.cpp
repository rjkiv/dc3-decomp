#include "meta_ham/PassiveMessenger.h"
#include "flow/PropertyEventProvider.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "utl/Locale.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

PassiveMessenger *ThePassiveMessenger;

#pragma region PassiveMessage

PassiveMessage::PassiveMessage(String text, PassiveMessageType type, Symbol channel, int i)
    : mText(text), mType(type), mChannel(channel), unk14(i) {}

PassiveMessage::~PassiveMessage() {}

String PassiveMessage::GetText() { return mText; }

#pragma endregion
#pragma region PassiveMessageQueue

PassiveMessageQueue::PassiveMessageQueue(Hmx::Object *callback, Symbol sym)
    : unk8(4000.0f), unkc(0), mCallback(callback), unk1c(sym) {}

PassiveMessageQueue::~PassiveMessageQueue() {
    FOREACH (it, mQueue) {
        delete *it;
    }
    mQueue.clear();
}

PassiveMessage *PassiveMessageQueue::GetAndPreProcessFirstMessage() {
    return mQueue.front();
}

void PassiveMessageQueue::AddMessage(PassiveMessage *msg) {
    Symbol symChannel = msg->Channel();
    if (symChannel == gNullStr || RemoveLowerPriorityMessage(msg)) {
        mQueue.push_back(msg);
    }
}

bool PassiveMessageQueue::HasRecentlyDismissedMessage() const {
    float seconds = TheTaskMgr.UISeconds() - unkc;
    if (seconds > 0)
        return 10.0f > seconds;
    return false;
}

void PassiveMessageQueue::Poll() {
    static Symbol is_message_hiding("is_message_hiding");
    static Symbol alert_target("alert_target");
    static Symbol alert_type("alert_type");
    static Symbol alert_text("alert_text");
    static Symbol alert_showing("alert_showing");

    if (mTimer.Running()) {
        if (mTimer.SplitMs() >= unk8) {
            mTimer.Stop();
            ClearPassiveMessage();
        }
    }
    if (!mTimer.Running()) {
        if (!mQueue.empty()) {
            PassiveMessage *message = GetAndPreProcessFirstMessage();
            MILO_ASSERT(message, 0x7e);
            HandlePassiveMessage(message);
            mQueue.pop_front();
            delete message;
            mTimer.Restart();
        }
    }
}

void PassiveMessageQueue::ClearRunningMessage() {
    if (mTimer.Running()) {
        mTimer.Stop();
        ClearPassiveMessage();
    }
}

bool PassiveMessageQueue::RemoveLowerPriorityMessage(PassiveMessage *msg) {
    Symbol symChannel = msg->Channel();
    MILO_ASSERT(symChannel != gNullStr, 0x10c);
    int i5 = msg->Unk14();
    FOREACH (it, mQueue) {
        PassiveMessage *queueMessage = *it;
        MILO_ASSERT(queueMessage, 0x115);
        if (queueMessage->Channel() == symChannel) {
            if (i5 >= queueMessage->Unk14())
                return false;
            else {
                PassiveMessage *pMessage = *it;
                MILO_ASSERT(pMessage, 0x124);
                delete pMessage;
                mQueue.erase(it);
                break;
            }
        }
    }
    return true;
}

void PassiveMessageQueue::ClearPassiveMessage() {
    static Symbol none("none");
    static Symbol p1("p1");
    static Symbol p2("p2");
    Symbol which = none;
    String str = "";
    unkc = TheTaskMgr.UISeconds();
    if (unk1c == none) {
        static Message cSetupAlertMsg("setup_alert", 0, 0);
        cSetupAlertMsg[0] = which;
        cSetupAlertMsg[1] = str;
        static Symbol setup_alert("setup_alert");
        TheHamProvider->Handle(cSetupAlertMsg, false);
    } else if (unk1c == p1) {
        static Message cSetup0AlertMsg("setup_0_alert", 0, 0);
        cSetup0AlertMsg[0] = which;
        cSetup0AlertMsg[1] = str;
        TheHamProvider->Handle(cSetup0AlertMsg, false);
    } else if (unk1c == p2) {
        static Message cSetup1AlertMsg("setup_1_alert", 0, 0);
        cSetup1AlertMsg[0] = which;
        cSetup1AlertMsg[1] = str;
        TheHamProvider->Handle(cSetup1AlertMsg, false);
    }
}

void PassiveMessageQueue::HandlePassiveMessage(PassiveMessage *msg) {
    static Symbol none("none");
    static Symbol p1("p1");
    static Symbol p2("p2");
    Symbol which = none;
    PassiveMessageType type = msg->Type();
    String text = msg->GetText();
    switch (type) {
    case kPassiveMessageGeneral: {
        static Symbol general("general");
        which = general;
        break;
    }
    case kPassiveMessageFitness: {
        static Symbol fitness("fitness");
        which = fitness;
        break;
    }
    case kPassiveMessageUnlock: {
        static Symbol unlock("unlock");
        which = unlock;
        break;
    }
    case kPassiveMessageServer: {
        static Symbol server("server");
        which = server;
        break;
    }
    case kPassiveMessageCampaign: {
        static Symbol campaign("campaign");
        which = campaign;
        break;
    }
    case kPassiveMessageFree4All: {
        static Symbol free4all("free4all");
        which = free4all;
        break;
    }
    default:
        MILO_ASSERT(false, 0xE2);
        break;
    }
    if (unk1c == none) {
        static Message cSetupAlertMsg("setup_alert", 0, 0);
        cSetupAlertMsg[0] = which;
        cSetupAlertMsg[1] = text;
        static Symbol setup_alert("setup_alert");
        TheHamProvider->Handle(cSetupAlertMsg, false);
    } else if (unk1c == p1) {
        static Message cSetup0AlertMsg("setup_0_alert", 0, 0);
        cSetup0AlertMsg[0] = which;
        cSetup0AlertMsg[1] = text;
        TheHamProvider->Handle(cSetup0AlertMsg, false);
    } else if (unk1c == p2) {
        static Message cSetup1AlertMsg("setup_1_alert", 0, 0);
        cSetup1AlertMsg[0] = which;
        cSetup1AlertMsg[1] = text;
        TheHamProvider->Handle(cSetup1AlertMsg, false);
    }
}

#pragma endregion
#pragma region PassiveMessenger

PassiveMessenger::PassiveMessenger(Hmx::Object *callback) {
    static Symbol none("none");
    static Symbol p1("p1");
    static Symbol p2("p2");
    mMessageQueueP1 = new PassiveMessageQueue(callback, p1);
    mMessageQueueP2 = new PassiveMessageQueue(callback, p2);
    mMessageQueueNone = new PassiveMessageQueue(callback, none);
    mEnabled = true;
    MILO_ASSERT(!ThePassiveMessenger, 0x159);
    ThePassiveMessenger = this;
    SetName("passive_messenger", ObjectDir::Main());
}

PassiveMessenger::~PassiveMessenger() {
    RELEASE(mMessageQueueP1);
    RELEASE(mMessageQueueP2);
    RELEASE(mMessageQueueNone);
    MILO_ASSERT(ThePassiveMessenger, 0x164);
    ThePassiveMessenger = nullptr;
}

BEGIN_HANDLERS(PassiveMessenger)
    HANDLE_ACTION(
        trigger_message,
        TriggerGenericMsg(_msg->Sym(2), _msg->Sym(3), kPassiveMessageGeneral, gNullStr, -1)
    )
    HANDLE_ACTION(
        trigger_campaign_message,
        TriggerGenericMsg(
            _msg->Sym(2), _msg->Sym(3), kPassiveMessageCampaign, gNullStr, -1
        )
    )
    HANDLE_EXPR(has_messages, HasMessages())
END_HANDLERS

BEGIN_PROPSYNCS(PassiveMessenger)
    SYNC_PROP(enabled, mEnabled)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void PassiveMessenger::Poll() {
    MILO_ASSERT(mMessageQueueP1, 0x16a);
    MILO_ASSERT(mMessageQueueP2, 0x16b);
    MILO_ASSERT(mMessageQueueNone, 0x16c);
    if (AreGlobalMessagesAllowed()) {
        mMessageQueueNone->Poll();
    } else {
        mMessageQueueNone->ClearRunningMessage();
    }
    if (AreSideMessagesAllowed()) {
        mMessageQueueP1->Poll();
        mMessageQueueP2->Poll();
    } else {
        mMessageQueueP1->ClearRunningMessage();
        mMessageQueueP2->ClearRunningMessage();
    }
}

bool PassiveMessenger::HasRecentlyDismissedMessage() const {
    if (mMessageQueueNone->HasRecentlyDismissedMessage()
        || mMessageQueueP1->HasRecentlyDismissedMessage()
        || mMessageQueueP2->HasRecentlyDismissedMessage())
        return true;
    else
        return false;
}

bool PassiveMessenger::AreSideMessagesAllowed() const {
    if (!mEnabled)
        return false;
    else {
        static Symbol ui_nav_mode("ui_nav_mode");
        const DataNode *pNavModeNode = TheHamProvider->Property(ui_nav_mode);
        MILO_ASSERT(pNavModeNode, 0x33);
        Symbol sym = pNavModeNode->Sym();
        static Symbol init("init");
        static Symbol game("game");
        static Symbol movie("movie");
        if (sym == init || sym == game || sym == movie)
            return false;
        else
            return true;
    }
}

bool PassiveMessenger::AreGlobalMessagesAllowed() const {
    if (!mEnabled)
        return false;
    else {
        static Symbol ui_nav_mode("ui_nav_mode");
        const DataNode *pNavModeNode = TheHamProvider->Property(ui_nav_mode);
        MILO_ASSERT(pNavModeNode, 0x4c);
        Symbol sym = pNavModeNode->Sym();
        static Symbol init("init");
        static Symbol title("title");
        static Symbol movie("movie");
        if (sym == init || sym == movie)
            return false;
        else
            return true;
    }
}

void PassiveMessenger::TriggerMessage(
    String str, PassiveMessageType pmt, Symbol s1, Symbol s2, int i
) {
    MILO_ASSERT(mMessageQueueP1, 0x186);
    MILO_ASSERT(mMessageQueueP2, 0x187);
    MILO_ASSERT(mMessageQueueNone, 0x188);
    static Symbol p1("p1");
    static Symbol p2("p2");
    static Symbol none("none");

    if (s1 == p1) {
        mMessageQueueP1->AddMessage(new PassiveMessage(str, pmt, s2, i));
    } else if (s1 == p2) {
        mMessageQueueP2->AddMessage(new PassiveMessage(str, pmt, s2, i));
    } else if (s1 == none) {
        mMessageQueueNone->AddMessage(new PassiveMessage(str, pmt, s2, i));
    } else
        MILO_ASSERT(false, 0x19a);
}

void PassiveMessenger::TriggerGenericMsg(
    Symbol s1, Symbol s2, PassiveMessageType pmt, Symbol s3, int i
) {
    String localize = Localize(s1, false, TheLocale);
    TriggerMessage(localize, pmt, s2, s3, i);
}

void PassiveMessenger::TriggerStringMsg(
    String str, Symbol s1, PassiveMessageType pmt, Symbol s2, int i
) {
    TriggerMessage(str, pmt, s1, s2, i);
}

bool PassiveMessenger::HasMessages() const {
    if (mMessageQueueNone->NumMessagesInQueue() > 0) {
        return true;
    }
    if (mMessageQueueP1->NumMessagesInQueue() > 0) {
        return true;
    }
    if (mMessageQueueP2->NumMessagesInQueue() > 0) {
        return true;
    }
    if (!mMessageQueueNone->Running() && !mMessageQueueP1->Running()
        && !mMessageQueueP2->Running())
        return false;
    return true;
}

#pragma endregion
