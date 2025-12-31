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
    HANDLE_EXPR(post_setup, 0)
    HANDLE_SUPERCLASS(UIPanel)
    HANDLE_MEMBER_PTR(unk38)
END_HANDLERS
