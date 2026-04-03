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
    RndGroup *r = LoadedDir()->Find<RndGroup>("progress.grp", true);
    r->SetFrame(0, 1.0f);
    ShowIfPossible();
}

void ContentLoadingPanel::ShowIfPossible() {
    if (!mShowing) {
        if (unk3c && (bool)(unk40 > 1)) {
            MILO_ASSERT(CheckIsLoaded(), 0x84);
            Enter();
            mShowing = true;
        }
    }
}

void ContentLoadingPanel::ContentDone() {
    unk40 = 0;
    unk44 = 0;
    unk3c = false;
    if (mState == 1 && mShowing) {
        Exit();
        mShowing = false;
    }
}

void ContentLoadingPanel::Poll() {
    ShowIfPossible();
    if (mShowing) {
        UIPanel::Poll();
        RndGroup *progressGroup = LoadedDir()->Find<RndGroup>("progress.grp");
        float frame = progressGroup->GetFrame();
        static float sFloat = 100.0f;
        float f;
        if (unk40 > 0) {
            f = unk44 * 110.0f / unk40;
        } else {
            f = sFloat;
        }
        if (f > sFloat) {
            f = sFloat;
        }
        float deltaSeconds = TheTaskMgr.DeltaSeconds();
        if (deltaSeconds < 0.0f) {
            deltaSeconds = 0.0f;
        } else if (deltaSeconds > 1.0f) {
            deltaSeconds = 1.0f;
        }
        float tempFrame = (f - frame) * deltaSeconds + frame;
        if (unk44 == unk40) {
            tempFrame = sFloat;
        }
        progressGroup->SetFrame(tempFrame, 1.0f);
    }
}

void ContentLoadingPanel::ContentMounted(char const *c1, char const *c2) {
    unk44++;
    ShowIfPossible();
}

void ContentLoadingPanel::ContentFailed(const char *) {
    unk44++;
    ShowIfPossible();
}

BEGIN_HANDLERS(ContentLoadingPanel)
    HANDLE_ACTION(allowed_to_show, AllowedToShow(_msg->Int(2)))
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
