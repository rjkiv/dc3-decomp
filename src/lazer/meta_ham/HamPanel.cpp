#include "lazer/meta_ham/HamPanel.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamNavList.h"
#include "lazer/meta_ham/HamUI.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIComponent.h"
#include "ui/UIPanel.h"

HamPanel::HamPanel() : mNavList(nullptr) {}

void HamPanel::Enter() {
    UIPanel::Enter();
    if (ShouldUseLocalNavlist()) {
        HamNavList *right_hand = DataDir()->Find<HamNavList>("right_hand.hnl", false);
        mNavList = right_hand;
        if (right_hand == nullptr) {
            HamNavList *left_hand = DataDir()->Find<HamNavList>("left_hand.hnl", false);
            mNavList = left_hand;
        }
    }
    static Symbol draw_after_letterbox("draw_after_letterbox");
    auto letterbox = Property(draw_after_letterbox, false);
    if (letterbox && letterbox->Int() != 0)
        unk34 = 1;
    else
        unk34 = 0;
}

bool HamPanel::Exiting() const {
    if (UIPanel::Exiting()) {
        return true;
    } else if (ShouldUseLocalNavlist() && mNavList) {
        return !mNavList->IsAnimating();
    }
    return false;
}

void HamPanel::Poll() { UIPanel::Poll(); }

UIComponent *HamPanel::FocusComponent() {
    auto pEventDialog = TheHamUI.EventDialogPanel();
    MILO_ASSERT(pEventDialog, 60);
    return UIPanel::FocusComponent();
}

BEGIN_HANDLERS(HamPanel)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
