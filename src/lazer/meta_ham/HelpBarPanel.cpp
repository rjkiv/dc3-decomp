#include "meta_ham/HelpBarPanel.h"
#include "HamPanel.h"
#include "HelpBarPanel.h"
#include "flow/Flow.h"
#include "gesture/GestureMgr.h"
#include "gesture/WaveToTurnOnLight.h"
#include "meta_ham/SaveLoadManager.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

HelpBarPanel::HelpBarPanel()
    : unk3c(0), unk40(0), unk44(false), unk78(false), unk79(true), unk7a(false),
      unk7b(false), unk7c(false), unkb0(0) {
    sInstance = this;
}

HelpBarPanel::~HelpBarPanel() { sInstance = nullptr; }

BEGIN_PROPSYNCS(HelpBarPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HelpBarPanel::Draw() {
    bool b = (!ShouldHideHelpbar() && !unk78) || TheGestureMgr->InControllerMode();
    unk40->SetShowing(b);
    UIPanel::Draw();
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
    unk7c = true;
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
    unk7c = false;
    Flow *f = DataDir()->Find<Flow>("end_wave_icon_display.flow", false);
    if (f)
        f->Activate();
}

void HelpBarPanel::PollSaveDeactivation() {
    if (unk44 && 1000.0f <= unk48.SplitMs()) {
        unk44 = false;
        DeactivatePhysicalWriteIcon();
    }
}

bool HelpBarPanel::ShouldHideHelpbar() const {
    static Symbol hide_helpbar("hide_helpbar");
    if (unkb0) {
        const DataNode *prop = unkb0->Property(hide_helpbar, false);
        if (prop && prop->Int()) {
            return true;
        }
    }
    return false;
}

DataNode HelpBarPanel::OnWaveGestureEnabled(DataArray const *) {
    ShowWaveGestureIcon();
    return DataNode(0);
}

DataNode HelpBarPanel::OnWaveGestureDisabled(DataArray const *) {
    HideWaveGestureIcon();
    return DataNode(0);
}

DataNode HelpBarPanel::OnEnterBlacklightMode(DataArray const *d) {
    static Symbol list_dir_resource("list_dir_resource");
    static Symbol label_carousel_blacklight("label_carousel_blacklight");
    unk3c->SetProperty(list_dir_resource, label_carousel_blacklight);
    return DataNode(0);
}

DataNode HelpBarPanel::OnExitBlacklightMode(DataArray const *d) {
    static Symbol list_dir_resource("list_dir_resource");
    static Symbol label_carousel("label_carousel");
    unk3c->SetProperty(list_dir_resource, label_carousel);
    return DataNode(0);
}

BEGIN_HANDLERS(HelpBarPanel)
    HANDLE_ACTION(resync, SyncToPanel(unk3c))
    HANDLE_ACTION(sync_to_panel, SyncToPanel(_msg->Obj<UIPanel>(2)))
    HANDLE_EXPR(get_helpbar_provider, unkb0->GetHelpbarProvider()) // incorrect variable
    // HANDLE_EXPR(is_write_icon_up, inlineFunc)
    HANDLE_ACTION(set_tertiary_labels, unkb0->SetProviderNavItemLabels(0, _msg->Array(2)))
    HANDLE(enter_blacklight_mode, OnEnterBlacklightMode)
    HANDLE(exit_blacklight_mode, OnExitBlacklightMode)
    HANDLE(wave_gesture_enable, OnWaveGestureEnabled)
    HANDLE(wave_gesture_disabled, OnWaveGestureDisabled)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_MESSAGE(SaveLoadMgrStatusUpdateMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
