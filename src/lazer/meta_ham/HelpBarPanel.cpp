#include "meta_ham/HelpBarPanel.h"
#include "flow/Flow.h"
#include "gesture/GestureMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
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
    // Need SaveLoadMgr
    UIPanel::Unload();
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

    return DataNode(0);
}

BEGIN_HANDLERS(HelpBarPanel)
END_HANDLERS
