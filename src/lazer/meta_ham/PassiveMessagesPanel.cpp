#include "meta_ham/PassiveMessagesPanel.h"
#include "PassiveMessenger.h"
#include "macros.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"

PassiveMessagesPanel::PassiveMessagesPanel() { unk38 = new PassiveMessenger(this); }

PassiveMessagesPanel::~PassiveMessagesPanel() { RELEASE(unk38); }

void PassiveMessagesPanel::Poll() {
    unk38->Poll();
    UIPanel::Poll();
}

BEGIN_HANDLERS(PassiveMessagesPanel)
    HANDLE_ACTION(post_setup, nullptr) // idk whats goin on with this one
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
