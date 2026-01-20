#include "meta_ham/SongSelectPanel.h"
#include "ContentLoadingPanel.h"
#include "SongSelectPanel.h"
#include "flow/Flow.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/HamUI.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "rndobj/Tex.h"
#include "ui/UIPanel.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

SongSelectPanel::SongSelectPanel() {}

void SongSelectPanel::Poll() { HamPanel::Poll(); }

void SongSelectPanel::Load() {
    UIPanel::Load();
    TheContentMgr.StartRefresh();
    ContentLoadingPanel *c =
        ObjectDir::Main()->Find<ContentLoadingPanel>("content_loading_panel", true);
    c->AllowedToShow(true);
}

void SongSelectPanel::Exit() {
    if (TheHamUI.GetLetterboxPanel()) {
        TheHamUI.GetLetterboxPanel()->RemoveSink(this, "enter_blacklight_mode");
        TheHamUI.GetLetterboxPanel()->RemoveSink(this, "exit_blacklight_mode");
    }
    UIPanel::Exit();
}

void SongSelectPanel::FinishLoad() {
    UIPanel::FinishLoad();
    ContentLoadingPanel *c =
        ObjectDir::Main()->Find<ContentLoadingPanel>("content_loading_panel", true);
    c->AllowedToShow(false);
}

bool SongSelectPanel::IsLoaded() const {
    if (TheContentMgr.RefreshDone())
        return UIPanel::IsLoaded();
    return false;
}

DataNode SongSelectPanel::OnEnterBlacklightMode(DataArray const *d) {
    Flow *f = DataDir()->Find<Flow>("activate_blacklight.flow", false);
    if (f)
        f->Activate();
    return DATA_UNHANDLED;
}

DataNode SongSelectPanel::OnExitBlacklightMode(DataArray const *d) {
    Flow *f = DataDir()->Find<Flow>("deactivate_blacklight.flow", false);
    if (f)
        f->Activate();
    return DATA_UNHANDLED;
}

RndTex *SongSelectPanel::GetTexForCharacter(Symbol s) {
    RndTex *tex = nullptr;
    static Symbol character_default("character_default");
    if (s != character_default) {
        String str = MakeString("portrait_purple_%s.tex", s.Str());
        if (DataDir()) {
            tex = DataDir()->Find<RndTex>(str.c_str(), false);
        }
    }
    return tex;
}

BEGIN_HANDLERS(SongSelectPanel)
    HANDLE_EXPR(
        is_valid_song, TheHamSongMgr.GetSongIDFromShortName(_msg->Sym(2), false) != false
    )
    HANDLE_EXPR(get_char_portrait_texture, GetTexForCharacter(_msg->Sym(2)))
    HANDLE(enter_blacklight_mode, OnEnterBlacklightMode)
    HANDLE(exit_blacklight_mode, OnExitBlacklightMode)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
