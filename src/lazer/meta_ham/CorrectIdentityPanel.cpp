#include "meta_ham/CorrectIdentityPanel.h"
#include "CorrectIdentityPanel.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/OverlayPanel.h"
#include "meta_ham/SkeletonIdentifier.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

CorrectIdentityPanel::CorrectIdentityPanel() {}

void CorrectIdentityPanel::Exit() { OverlayPanel::Exit(); }

void CorrectIdentityPanel::Dismiss() {
    MILO_ASSERT(TheHamUI.GetOverlayPanel() == this, 0x67);
    TheHamUI.SetOverlayPanel(nullptr);
    OverlayPanel::Dismiss();
}

void CorrectIdentityPanel::SetAsOverlay() {
    if (TheHamUI.GetOverlayPanel() && TheHamUI.GetOverlayPanel() != this) {
        TheHamUI.GetOverlayPanel()->Dismiss();
    }
    MILO_ASSERT(this->CheckIsLoaded(), 0x37);
    MILO_ASSERT(this->LoadedDir(), 0x38);
    TheHamUI.SetOverlayPanel(this);
}

void CorrectIdentityPanel::Enter() {
    UpdateIdentityList();
    OverlayPanel::Enter();
}

bool CorrectIdentityPanel::IdentitySelected(int idx) {
    int identity = mIdentityList[idx];
    if (identity == -1) {
        ThePlatformMgr.SignInUsers(1, 0x1000000);
        return false;
    } else {
        TheSkeletonIdentifier->CorrectIdentity(identity);
        return true;
    }
}

void CorrectIdentityPanel::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mIdentityList.size(), 0x52);
    static Symbol enroll_sign_in("enroll_sign_in");
    static Symbol new_player("new_player");
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x57);
    int identity = mIdentityList[data];
    if (uiListLabel->Matches("gamertag")) {
        if (identity == -1) {
            app_label->SetTextToken(enroll_sign_in);
        } else if (identity == -3) {
            app_label->SetTextToken(new_player);
        } else {
            app_label->SetEnrolledPlayerName(identity);
        }
    }
}

void CorrectIdentityPanel::UpdateIdentityList() {
    mIdentityList.clear();
    for (int i = 0; i < 8; i++) {
        String playerName = TheSkeletonIdentifier->GetPlayerName(i);
        static Symbol signing_in("signing_in");
        if (TheSkeletonIdentifier->IsAssociatedWithProfile(i)) {
            if (playerName != Localize(signing_in, false, TheLocale)) {
                mIdentityList.push_back(i);
            }
        }
    }
    mIdentityList.push_back(-3);
    mIdentityList.push_back(-1);
}

BEGIN_HANDLERS(CorrectIdentityPanel)
    HANDLE_EXPR(identity_selected, IdentitySelected(_msg->Int(2)))
    HANDLE_ACTION(set_as_overlay, SetAsOverlay())
    HANDLE_ACTION(dismiss, Dismiss())
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(OverlayPanel)
END_HANDLERS
