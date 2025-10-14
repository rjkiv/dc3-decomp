#include "meta/PreloadPanel.h"
#include "SongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

PreloadPanel::PreloadPanel()
    : unk3c(kPreloadInProgress), unk4c(0), unk5c(), unk60(0), unk6c(0) {}

PreloadPanel::~PreloadPanel() {}

void PreloadPanel::Load() {}

void PreloadPanel::SetTypeDef(DataArray *d) {
    UIPanel::SetTypeDef(d);
    CheckTypeDef("max_cache_size");
    CheckTypeDef("song_mgr");
    CheckTypeDef("current_song");
    CheckTypeDef("on_preload_ok");
    CheckTypeDef("preload_files");
}

bool PreloadPanel::IsLoaded() const {
    if (!UIPanel::IsLoaded())
        return false;
    else
        return unk3c != kPreloadInProgress;
}

void PreloadPanel::Unload() {
    unk50.clear();
    UIPanel::Unload();
}

void PreloadPanel::PollForLoading() {
    UIPanel::PollForLoading();
    if (UIPanel::IsLoaded()) {
        if (!unk4c && unk50.empty()) {
            StartCache();
        }
    }
}

void PreloadPanel::FinishLoad() {
    UIPanel::FinishLoad();
    TheContentMgr->UnregisterCallback(this, true);
    ClearAndShrink(unk40);
    TheContentMgr->SetReadFailureHandler(unk5c);
}

void PreloadPanel::ContentMounted(char const *c1, char const *c2) {
    OnContentMountedOrFailed(c1);
}

void PreloadPanel::ContentFailed(char const *c) { OnContentMountedOrFailed(c); }

Symbol PreloadPanel::CurrentSong() const {
    static Symbol current_song("current_song");
    return TypeDef()->FindSym(current_song);
}

void PreloadPanel::CheckTypeDef(Symbol s) {
    if (!TypeDef()->FindArray(s, false))
        MILO_NOTIFY(
            "PreloadPanel %s missing %s handler (%s)", Name(), s, TypeDef()->File()
        );
}

bool PreloadPanel::CheckFileCached(char const *) { return false; }

SongMgr *PreloadPanel::FindSongMgr() const {
    static Symbol song_mgr("song_mgr");
    return TypeDef()->FindArray(song_mgr)->Obj<SongMgr>(1);
}

DataNode PreloadPanel::OnMsg(ContentReadFailureMsg const &msg) {
    unk60 = msg->Int(2);
    unk64 = msg->Str(3);
    return 1;
}

// DataNode PreloadPanel::OnMsg(UiTransitionCompleteMsg const &){}

void PreloadPanel::OnContentMountedOrFailed(char const *contentName) {
    if (!unk50.empty()) {
        MILO_ASSERT(contentName, 0x12b);
        for (std::vector<Symbol>::iterator it = unk50.begin(); it != unk50.end();) {
            Symbol s = *it;
            if (s == contentName) {
                it = unk50.erase(it);
            } else {
                it++;
            }
        }
    }
}

void PreloadPanel::StartCache() {}

BEGIN_HANDLERS(PreloadPanel)
    HANDLE_MESSAGE(ContentReadFailureMsg)
    // HANDLE_MESSAGE(UITransitionCompleteMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
