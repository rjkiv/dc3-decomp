#include "meta_ham/ContentLoadingPanel.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "rndobj/Group.h"
#include "ui/PanelDir.h"
#include "ui/UIPanel.h"

ContentLoadingPanel::ContentLoadingPanel() : unk3c(false), unk40(0), unk44(0) {
    TheContentMgr.RegisterCallback(this, false);
    mShowing = false;
}

ContentLoadingPanel::~ContentLoadingPanel() {
    TheContentMgr.UnregisterCallback(this, false);
}

void ContentLoadingPanel::AllowedToShow(bool b) {
    unk3c = b;
    if (b) {
        ShowIfPossible();
    }
}

void ContentLoadingPanel::ContentMountBegun(int i) {
    unk40 = i;
    unk44 = 0;
    RndGroup *r = GetPanelDir()->Find<RndGroup>("progress.grp", true);
    r->SetFrame(0, 1.0f);
    ShowIfPossible();
}

void ContentLoadingPanel::ShowIfPossible() {
    if (!mShowing) {
        if (unk3c) { // theres an extra check here
            MILO_ASSERT(CheckIsLoaded(), 0x84);
            Enter();
            mShowing = true;
        }
    }
}

BEGIN_HANDLERS(ContentLoadingPanel)
    HANDLE_ACTION_IF(allowed_to_show, unk3c != _msg->Int(2), ShowIfPossible())
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
