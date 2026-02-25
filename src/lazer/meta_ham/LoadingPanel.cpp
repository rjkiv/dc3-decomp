#include "meta_ham/LoadingPanel.h"
#include "game/SongDB.h"
#include "hamobj/HamAudio.h"
#include "hamobj/HamMaster.h"
#include "macros.h"
#include "meta/DataArraySongInfo.h"
#include "meta_ham/ContextChecker.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/FileCache.h"
#include "os/System.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/BeatMap.h"
#include "utl/MakeString.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include "utl/TempoMap.h"
#include "utl/TimeConversion.h"

LoadingPanel::LoadingPanel() : unk38(0), unk3c(), unk40(0) { sSongDB = new SongDB(); }

LoadingPanel::~LoadingPanel() {
    RELEASE(sSongDB);
    RELEASE(sLoadingMaster);
}

BEGIN_PROPSYNCS(LoadingPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

char const *LoadingPanel::GetLoadingScreen(Symbol s) {
    DataArray *screenArray = SystemConfig("loading_screens");
    for (int i = 1; i < screenArray->Size(); i++) {
        DataArray *loadingScreenArray = screenArray->Array(i);
        if (loadingScreenArray->Sym(0) == s) {
            return loadingScreenArray->Str(1);
        }
    }
    MILO_FAIL("can\'t find loadingScreen %s", s);
    return "";
}

void LoadingPanel::Unload() {
    if (unk3c)
        SetTheTempoMap(unk3c);

    if (unk40)
        SetTheBeatMap(unk40);

    delete sLoadingMaster;
    UIPanel::Unload();
}

void LoadingPanel::Load() {
    UIPanel::Load();
    sLoadingMaster = new HamMaster(sSongDB->SongData(), nullptr);
    PlayLoadingMusic();
    sLoadingMaster->SetMaps();
}

bool LoadingPanel::IsLoaded() const {
    HamAudio *pAudio = sLoadingMaster->GetAudio();
    if (!pAudio)
        MILO_NOTIFY("missing audio object!\n");

    return (
        TheContentMgr.RefreshDone() && UIPanel::IsLoaded() && pAudio && !pAudio->Fail()
        && !pAudio->IsReady()
    );
}

bool LoadingPanel::Exiting() {
    if (mState == kDown && !TheUI->TransitionScreen()->IsLoaded()) {
        return true;
    }
    return UIPanel::Exiting();
}

void LoadingPanel::Enter() {
    UIPanel::Enter();
    TheTaskMgr.SetSecondsAndBeat(0, 0, true);
    MILO_ASSERT(sLoadingMaster->GetHxAudio()->IsReady(), 0x6a);
}

Symbol LoadingPanel::ChooseLoadingScreen() {
    Symbol randomItem =
        RandomContextSensitiveItem(SystemConfig("loading_screen_context"));
    return GetLoadingScreen(randomItem);
}

void LoadingPanel::PlayLoadingMusic() {
    static DataNode &n = DataVariable("loading_music_mogg");
    if (n.Equal(gNullStr, nullptr, true)) {
        ResetLoadingMusic();
    }
    const char *fileBase = FileGetBase(n.Str());
    {
        String filePath = MakeString("sfx/samples/shell/%s.mid", fileBase);
        File *f = FileCache::GetFileAll(filePath.c_str());
        MILO_ASSERT(f != NULL, 0xb7);
        delete f;
    }
    if (unk38)
        RELEASE(unk38);
    DataArray *sysConfig = SystemConfig("synth", fileBase);
    static Symbol song("song");
    DataArray *songArray = sysConfig->FindArray(song, false);
    unk38 = new DataArraySongInfo(songArray, nullptr, "loadmusic");
    sLoadingMaster->Load(unk38, false, 0, false, (HamSongDataValidate)0, nullptr);
}

BEGIN_HANDLERS(LoadingPanel)
    HANDLE_EXPR(choose_loading_screen, ChooseLoadingScreen())
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

void ResetLoadingMusic() {
    static Symbol reset_loading_music_mogg("reset_loading_music_mogg");
    static DataArrayPtr func(new DataArray(1));
    func.Node(0) = reset_loading_music_mogg;
    func->Execute(false);
}
