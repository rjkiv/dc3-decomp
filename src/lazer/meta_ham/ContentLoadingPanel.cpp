#include "meta_ham/ContentLoadingPanel.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "rndobj/Group.h"
#include "ui/PanelDir.h"
#include "ui/UIPanel.h"

ContentLoadingPanel::ContentLoadingPanel()
    : mAllowedToShow(false), mCount(0), mCurrent(0) {
    TheContentMgr.RegisterCallback(this, false);
    mShowing = false;
}

ContentLoadingPanel::~ContentLoadingPanel() {
    TheContentMgr.UnregisterCallback(this, false);
}

void ContentLoadingPanel::AllowedToShow(bool b) {
    mAllowedToShow = b;
    if (b) {
        ShowIfPossible();
    }
}

void ContentLoadingPanel::ContentMountBegun(int i) {
    mCount = i;
    mCurrent = 0;
    RndGroup *r = LoadedDir()->Find<RndGroup>("progress.grp", true);
    r->SetFrame(0, 1.0f);
    ShowIfPossible();
}

void ContentLoadingPanel::ShowIfPossible() {
    if (!mShowing) {
        if (mAllowedToShow && (bool)(mCount > 1)) {
            MILO_ASSERT(CheckIsLoaded(), 0x84);
            Enter();
            mShowing = true;
        }
    }
}

void ContentLoadingPanel::ContentDone() {
    mCount = 0;
    mCurrent = 0;
    mAllowedToShow = false;
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
        if (mCount > 0) {
            f = mCurrent * 110.0f / mCount;
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
        if (mCurrent == mCount) {
            tempFrame = sFloat;
        }
        progressGroup->SetFrame(tempFrame, 1.0f);
    }
}

void ContentLoadingPanel::ContentMounted(char const *c1, char const *c2) {
    mCurrent++;
    ShowIfPossible();
}

void ContentLoadingPanel::ContentFailed(const char *) {
    mCurrent++;
    ShowIfPossible();
}

BEGIN_HANDLERS(ContentLoadingPanel)
    HANDLE_ACTION(allowed_to_show, AllowedToShow(_msg->Int(2)))
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
