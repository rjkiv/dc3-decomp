#include "meta_ham/BlacklightPanel.h"
#include "flow/Flow.h"
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "ui/PanelDir.h"

BlacklightPanel::BlacklightPanel() { sInstance = this; }

BlacklightPanel::~BlacklightPanel() { sInstance = nullptr; }

BEGIN_PROPSYNCS(BlacklightPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void BlacklightPanel::Enter() { HamPanel::Enter(); } // incomplete

DataNode BlacklightPanel::OnEnterBlacklightMode(DataArray const *d) {
    Flow *f = DataDir()->Find<Flow>("activate_letterbox.flow", false);
    if (f)
        f->Activate();
    return DataNode(0);
}

DataNode BlacklightPanel::OnExitBlacklightMode(DataArray const *d) {
    Flow *f;
    if (d->Int(2) == 0) {
        f = DataDir()->Find<Flow>("deactivate_letterbox.flow", false);
    } else {
        f = DataDir()->Find<Flow>("deactivate_letterbox_immediate.flow", false);
    }
    if (f)
        f->Activate();
    return DataNode(0);
}

BEGIN_HANDLERS(BlacklightPanel)
    HANDLE(enter_blacklight_mode, OnEnterBlacklightMode)
    HANDLE(exit_blacklight_mode, OnExitBlacklightMode)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
