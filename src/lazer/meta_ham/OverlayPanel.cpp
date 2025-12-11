#include "meta_ham/OverlayPanel.h"
#include "HamUI.h"
#include "OverlayPanel.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/ShellInput.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"
#include "ui/UIPanel.h"

OverlayPanel::OverlayPanel() : mPostProc(0) {}

OverlayPanel::~OverlayPanel() {}

void OverlayPanel::Draw() {
    if (mState != kUp) {
        Enter();
        TheHamUI.unk_0x104->SyncToCurrentScreen();
    } else {
        UIPanel::Draw();
    }
}

void OverlayPanel::Enter() {
    mPostProc = TheRnd.GetPostProcOverride();
    UIPanel::Enter();
}

void OverlayPanel::Exit() {
    TheRnd.SetPostProcOverride(mPostProc);
    UIPanel::Exit();
}

void OverlayPanel::Dismiss() {
    if (mState != kDown) {
        Exit();
        TheHamUI.unk_0x104->SyncToCurrentScreen();
        static Message overlay_panel_dismissed("overlay_panel_dismissed");
        TheHamUI.Handle(overlay_panel_dismissed, false);
    }
}

BEGIN_HANDLERS(OverlayPanel)
    HANDLE_ACTION(dismiss, Dismiss())
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
