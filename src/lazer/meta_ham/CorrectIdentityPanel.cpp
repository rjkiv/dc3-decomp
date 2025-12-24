#include "meta_ham/CorrectIdentityPanel.h"
#include "CorrectIdentityPanel.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/OverlayPanel.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

CorrectIdentityPanel::CorrectIdentityPanel() {}

void CorrectIdentityPanel::Exit() { OverlayPanel::Exit(); }

void CorrectIdentityPanel::Dismiss() {
    MILO_ASSERT(TheHamUI.GetOverlayPanel() == this, 0x67);
    TheHamUI.SetOverlayPanel(nullptr);
    OverlayPanel::Dismiss();
}

void CorrectIdentityPanel::SetAsOverlay() {
    TheHamUI.SetOverlayPanel(this);
    MILO_ASSERT(this->CheckIsLoaded(), 0x37);
    MILO_ASSERT(this->LoadedDir(), 0x38);
}

void CorrectIdentityPanel::Enter() {
    UpdateIdentityList();
    OverlayPanel::Enter();
}

BEGIN_HANDLERS(CorrectIdentityPanel)
    HANDLE_EXPR(identity_selected, IdentitySelected(_msg->Int(2)))
    HANDLE_ACTION(set_as_overlay, SetAsOverlay())
    HANDLE_ACTION(dismiss, Dismiss())
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(OverlayPanel)
END_HANDLERS
