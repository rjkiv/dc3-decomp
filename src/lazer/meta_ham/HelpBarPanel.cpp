#include "meta_ham/HelpBarPanel.h"
#include "HamPanel.h"
#include "HelpBarPanel.h"
#include "flow/Flow.h"
#include "gesture/GestureMgr.h"
#include "gesture/WaveToTurnOnLight.h"
#include "hamobj/HamNavList.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/SaveLoadManager.h"
#include "meta_ham/ShellInput.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Group.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

HelpBarPanel::HelpBarPanel()
    : mNavList(0), mHelpbarGroup(0), mIsSaveDeactivationQueued(false), mNoButtons(false),
      mControllerModeAllowed(true), mIsPhysicalWriteActive(false), mIsWriteIconUp(false),
      mIsWaveGestureIconUp(false), mSyncedPanel(0) {
    sInstance = this;
}

HelpBarPanel::~HelpBarPanel() { sInstance = nullptr; }

BEGIN_HANDLERS(HelpBarPanel)
    HANDLE_ACTION(resync, SyncToPanel(mSyncedPanel))
    HANDLE_ACTION(sync_to_panel, SyncToPanel(_msg->Obj<UIPanel>(2)))
    HANDLE_EXPR(get_helpbar_provider, mNavList->GetHelpbarProvider())
    HANDLE_EXPR(is_write_icon_up, IsWriteIconUp())
    HANDLE_ACTION(set_tertiary_labels, SetTertiaryLabels(_msg->Array(2)))
    HANDLE(enter_blacklight_mode, OnEnterBlacklightMode)
    HANDLE(exit_blacklight_mode, OnExitBlacklightMode)
    HANDLE(wave_gesture_enabled, OnWaveGestureEnabled)
    HANDLE(wave_gesture_disabled, OnWaveGestureDisabled)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_MESSAGE(SaveLoadMgrStatusUpdateMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

BEGIN_PROPSYNCS(HelpBarPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HelpBarPanel::Draw() {
    bool show =
        (!ShouldHideHelpbar() && !mNoButtons) || TheGestureMgr->InControllerMode();
    mHelpbarGroup->SetShowing(show);
    UIPanel::Draw();
}

void HelpBarPanel::Enter() {
    MILO_LOG("%s %d\n", "HelpBarPanel::Enter", 0x48);
    mNavList = DataDir()->Find<HamNavList>("left_hand.hnl", false);
    mHelpbarGroup = DataDir()->Find<RndGroup>("all.grp");
    TheSaveLoadMgr->AddSink(this);
    if (TheHamUI.Letterbox()) {
        TheHamUI.Letterbox()->AddSink(this, "enter_blacklight_mode");
        TheHamUI.Letterbox()->AddSink(this, "exit_blacklight_mode");
        for (ObjDirItr<HamNavList> it(DataDir(), true); it != nullptr; ++it) {
            it->AddRibbonSinks(TheHamUI.Letterbox(), "enter_blacklight_mode");
            it->AddRibbonSinks(TheHamUI.Letterbox(), "exit_blacklight_mode");
        }
    }
    HamPanel::Enter();
}

void HelpBarPanel::Exit() {
    MILO_LOG("%s %d\n", "HelpBarPanel::Exit", 0x64);
    TheSaveLoadMgr->RemoveSink(this);
    if (TheHamUI.Letterbox()) {
        TheHamUI.Letterbox()->RemoveSink(this, "enter_blacklight_mode");
        TheHamUI.Letterbox()->RemoveSink(this, "exit_blacklight_mode");
        for (ObjDirItr<HamNavList> it(DataDir(), true); it != nullptr; ++it) {
            it->RemoveRibbonSinks(TheHamUI.Letterbox(), "enter_blacklight_mode");
            it->RemoveRibbonSinks(TheHamUI.Letterbox(), "exit_blacklight_mode");
        }
    }
    HamPanel::Exit();
}

void HelpBarPanel::Poll() {
    HamPanel::Poll();
    if (ShouldHideHelpbar() && mNavList->Enabled()) {
        mNavList->Disable();
    } else if (!ShouldHideHelpbar() && !mNavList->Enabled() && !mNoButtons) {
        mNavList->Enable();
    }
    if (mIsWriteIconUp && !mIsPhysicalWriteActive && mTimerWrite.SplitMs() >= 3000.0f) {
        mIsWriteIconUp = false;
        HidePhysicalWriteIcon();
        if (!mIsSaveDeactivationQueued) {
            mIsSaveDeactivationQueued = true;
            mSaveTimerActive.Restart();
        }
    }
    PollSaveDeactivation();
}

void HelpBarPanel::Unload() {
    TheSaveLoadMgr->RemoveSink(this);
    UIPanel::Unload();
}

void HelpBarPanel::FinishLoad() {
    UIPanel::FinishLoad();
    TheSaveLoadMgr->AddSink(this);
    TheWaveToTurnOnLight->AddSink(this, "wave_gesture_enabled");
    TheWaveToTurnOnLight->AddSink(this, "wave_gesture_disabled");
}

void HelpBarPanel::EnterControllerMode() {
    Flow *f = DataDir()->Find<Flow>("controller_mode.flow", false);
    if (f)
        f->Activate();
}

void HelpBarPanel::ExitControllerMode(bool b) {
    Flow *f;
    if (!b) {
        f = DataDir()->Find<Flow>("exit_controller_mode.flow", false);
    } else {
        f = DataDir()->Find<Flow>("exit_controller_mode_immediate.flow", false);
    }
    if (f)
        f->Activate();
}

void HelpBarPanel::ShowWaveGestureIcon() {
    MILO_LOG("HelpBarPanel: <<<OnWaveGestureEnabled>>>\n");
    mIsWaveGestureIconUp = true;
    Flow *f = DataDir()->Find<Flow>("start_wave_icon_display.flow", false);
    if (f)
        f->Activate();
}

void HelpBarPanel::ShowPhysicalWriteIcon() {
    Flow *f1 = DataDir()->Find<Flow>("autosave_activate.flow", false);
    if (f1)
        f1->Activate();

    Flow *f2 = DataDir()->Find<Flow>("start_autosave.flow", false);
    if (f2)
        f2->Activate();
}

void HelpBarPanel::HidePhysicalWriteIcon() {
    Flow *f = DataDir()->Find<Flow>("end_autosave.flow", false);
    if (f)
        f->Activate();
}

void HelpBarPanel::DeactivatePhysicalWriteIcon() {
    Flow *f = DataDir()->Find<Flow>("autosave_deactivate.flow", false);
    if (f)
        f->Activate();
}

void HelpBarPanel::HideWaveGestureIcon() {
    MILO_LOG("HelpBarPanel: <<<OnWaveGestureDisabled>>>\n");
    mIsWaveGestureIconUp = false;
    Flow *f = DataDir()->Find<Flow>("end_wave_icon_display.flow", false);
    if (f)
        f->Activate();
}

void HelpBarPanel::PollSaveDeactivation() {
    if (mIsSaveDeactivationQueued && 1000.0f <= mSaveTimerActive.SplitMs()) {
        mIsSaveDeactivationQueued = false;
        DeactivatePhysicalWriteIcon();
    }
}

bool HelpBarPanel::ShouldHideHelpbar() const {
    static Symbol hide_helpbar("hide_helpbar");
    if (mSyncedPanel) {
        const DataNode *prop = mSyncedPanel->Property(hide_helpbar, false);
        if (prop && prop->Int()) {
            return true;
        }
    }
    return false;
}

bool HelpBarPanel::IsWriteIconUp() const {
    return mIsWriteIconUp ? true : mIsSaveDeactivationQueued != false;
}

void HelpBarPanel::SetTertiaryLabels(DataArray *a) {
    if (mNavList) {
        mNavList->SetProviderNavItemLabels(0, a);
    }
}

bool HelpBarPanel::IsAnimating() {
    if (mNavList && mNavList->Enabled() && mNavList->IsAnimating()) {
        return true;
    } else {
        return false;
    }
}

bool HelpBarPanel::IsWriteIconShowing() {
    if (IsWriteIconUp()) {
        return true;
    } else {
        Flow *flow = DataDir()->Find<Flow>("autosave_deactivate.flow", false);
        if (flow && flow->IsRunning()) {
            return true;
        } else {
            return false;
        }
    }
}

void HelpBarPanel::SyncToPanel(UIPanel *panel) {
    mSyncedPanel = panel;
    bool updateback = UpdateBackButton(panel);
    bool updatetert = UpdateTertiaryButton(panel);
    if (updateback || updatetert) {
        if (mNavList) {
            mNavList->Enable();
        }
        mNoButtons = false;
    } else {
        if (mNavList) {
            mNavList->Disable();
        }
        mNoButtons = true;
    }
    const DataNode *prop = nullptr;
    if (mNavList) {
        if (!updatetert || updateback) {
            mNavList->SetHighButtonMode(false);
        } else {
            mNavList->SetHighButtonMode(true);
        }
    }
    static Symbol helpbar_confirm_label("helpbar_confirm_label");
    static Symbol helpbar_allow_controller("helpbar_allow_controller");
    mControllerModeAllowed = true;

    if (!panel) {
    ugh:
        if (TheHamUI.CurrentScreen()) {
            prop = TheHamUI.CurrentScreen()->Property(helpbar_confirm_label, false);
        }
    } else {
        prop = panel->Property(helpbar_confirm_label, false);
        const DataNode *allowProp = panel->Property(helpbar_allow_controller, false);
        if (allowProp) {
            mControllerModeAllowed = allowProp->Int();
        }
        if (!prop) {
            goto ugh;
        }
    }

    if (!mControllerModeAllowed) {
        ShellInput *pShellInput = TheHamUI.GetShellInput();
        MILO_ASSERT(pShellInput, 0xE2);
        pShellInput->ExitControllerMode(true);
    }
    static Symbol none("none");
    if (prop) {
        Symbol key = prop->Sym();
        if (key == none) {
            DataDir()->Find<UILabel>("select.lbl")->SetTextToken(gNullStr);
            DataDir()->Find<UILabel>("select_icon.lbl")->SetShowing(false);
        } else {
            DataDir()->Find<UILabel>("select.lbl")->SetTextToken(prop->Sym());
            DataDir()->Find<UILabel>("select_icon.lbl")->SetShowing(true);
        }
    } else {
        static Symbol help_select("help_select");
        DataDir()->Find<UILabel>("select.lbl")->SetTextToken(help_select);
        DataDir()->Find<UILabel>("select_icon.lbl")->SetShowing(true);
    }
}

bool HelpBarPanel::UpdateBackButton(UIPanel *panel) {
    static Symbol back_token("back_token");
    const DataNode *prop = nullptr;
    RndGroup *backIcon;
    if (panel) {
        prop = panel->Property(back_token, false);
        backIcon = DataDir()->Find<RndGroup>("back_icon.grp", false);
    } else {
        backIcon = DataDir()->Find<RndGroup>("back_icon.grp", false);
    }
    UILabel *leftHandLabel = DataDir()->Find<UILabel>("left_hand.lbl", false);
    bool b11 = false;
    if (prop) {
        if (prop->Type() == kDataSymbol) {
            if (prop->Sym() != gNullStr) {
                b11 = true;
                mNavList->GetHelpbarProvider()->SetLabel(1, 0, prop->Sym());
            }
        } else if (prop->Type() == kDataArray) {
            if (prop->Array()->Size() > 0) {
                b11 = true;
                mNavList->GetHelpbarProvider()->SetLabels(1, prop->Array());
            }
        }
    }
    if (b11) {
        mNavList->HideItem(1, false);
        mNavList->GetHelpbarProvider()->SetLabel(1, prop->Sym());
        if (backIcon) {
            static Symbol show_back_controller_icon("show_back_controller_icon");
            if (panel) {
                prop = panel->Property(show_back_controller_icon, false);
                if (!prop || prop->Int() != 0) {
                    backIcon->SetShowing(true);
                } else {
                    backIcon->SetShowing(false);
                }
            } else {
                backIcon->SetShowing(true);
            }
        }
        if (leftHandLabel) {
            leftHandLabel->SetShowing(true);
        }
        return true;
    } else {
        if (mNavList) {
            mNavList->HideItem(1, true);
        }
        if (backIcon) {
            backIcon->SetShowing(false);
        }
        if (leftHandLabel) {
            leftHandLabel->SetShowing(false);
        }
        return false;
    }
}

bool HelpBarPanel::UpdateTertiaryButton(UIPanel *panel) {
    static Symbol tertiary_token("tertiary_token");
    const DataNode *prop = nullptr;
    if (panel) {
        prop = panel->Property(tertiary_token, false);
    }
    bool b8 = false;
    RndGroup *iconGroup = DataDir()->Find<RndGroup>("tertiary_icon.grp", false);
    if (prop) {
        if (prop->Type() == kDataSymbol) {
            if (prop->Sym() != gNullStr) {
                b8 = true;
                mNavList->GetHelpbarProvider()->ResetLabelProvider(0);
                mNavList->GetHelpbarProvider()->SetLabel(0, 0, prop->Sym());
            }
        } else if (prop->Type() == kDataArray) {
            if (prop->Array()->Size() > 0) {
                b8 = true;
                mNavList->GetHelpbarProvider()->SetLabels(0, prop->Array());
            }
        }
    }
    if (mNavList) {
        mNavList->HideItem(0, !b8);
    }
    if (iconGroup) {
        iconGroup->SetShowing(b8);
    }
    DataDir()->Find<RndMesh>("behind_bar_top.mesh")->SetShowing(b8);
    return b8;
}

DataNode HelpBarPanel::OnWaveGestureEnabled(const DataArray *) {
    ShowWaveGestureIcon();
    return 0;
}

DataNode HelpBarPanel::OnWaveGestureDisabled(const DataArray *) {
    HideWaveGestureIcon();
    return 0;
}

DataNode HelpBarPanel::OnEnterBlacklightMode(const DataArray *d) {
    static Symbol list_dir_resource("list_dir_resource");
    static Symbol label_carousel_blacklight("label_carousel_blacklight");
    mNavList->SetProperty(list_dir_resource, label_carousel_blacklight);
    return 0;
}

DataNode HelpBarPanel::OnExitBlacklightMode(const DataArray *d) {
    static Symbol list_dir_resource("list_dir_resource");
    static Symbol label_carousel("label_carousel");
    mNavList->SetProperty(list_dir_resource, label_carousel);
    return 0;
}

DataNode HelpBarPanel::OnMsg(const ButtonDownMsg &msg) {
    if (TheGestureMgr->InControllerMode()) {
        if (msg.GetButton() == kPad_Xbox_Y
            && mNavList->GetHelpbarProvider()->IsEnabled(0)) {
            mNavList->DoSelectFor(0);
        }
    }
    return DATA_UNHANDLED;
}

DataNode HelpBarPanel::OnMsg(const SaveLoadMgrStatusUpdateMsg &msg) {
    switch (msg.GetStatus()) {
    case 1:
        if (mIsSaveDeactivationQueued) {
            mIsSaveDeactivationQueued = false;
        }
        if (!mIsPhysicalWriteActive) {
            mIsPhysicalWriteActive = true;
            mIsWriteIconUp = true;
            mTimerWrite.Restart();
            ShowPhysicalWriteIcon();
        }
        break;
    case 2:
    case 5:
        mIsPhysicalWriteActive = false;
        break;
    default:
        break;
    }
    return 0;
}
