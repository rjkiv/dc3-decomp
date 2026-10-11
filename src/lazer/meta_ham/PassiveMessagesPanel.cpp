#include "meta_ham/PassiveMessagesPanel.h"
#include "PassiveMessenger.h"
#include "macros.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"

PassiveMessagesPanel::PassiveMessagesPanel() { mMessenger = new PassiveMessenger(this); }

PassiveMessagesPanel::~PassiveMessagesPanel() { RELEASE(mMessenger); }

void PassiveMessagesPanel::Poll() {
    mMessenger->Poll();
    UIPanel::Poll();
}

BEGIN_HANDLERS(PassiveMessagesPanel)
    HANDLE_EXPR(post_setup, 0)
    HANDLE_SUPERCLASS(UIPanel)
    HANDLE_MEMBER_PTR(mMessenger)
END_HANDLERS
