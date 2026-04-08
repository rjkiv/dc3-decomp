#include "meta_ham/MetaPanel.h"
#include "FitnessCalorieSortMgr.h"
#include "HamProfile.h"
#include "HamScreen.h"
#include "TexLoadPanel.h"
#include "game/SongDB.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamSongData.h"
#include "macros.h"
#include "math/Rand.h"
#include "meta/CreditsPanel.h"
#include "meta/HAQManager.h"
#include "meta/MemcardMgr.h"
#include "meta/Meta.h"
#include "meta/MetaMusicManager.h"
#include "meta/MoviePanel.h"
#include "meta_ham/AppNavProvider.h"
#include "meta_ham/AccomplishmentProgress.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/AppMiniLeaderboardDisplay.h"
#include "meta_ham/BlacklightPanel.h"
#include "meta_ham/CalibrationPanel.h"
#include "meta_ham/CampaignDiffSelectPanel.h"
#include "meta_ham/CampaignMasterQuestCrewSelectPanel.h"
#include "meta_ham/CampaignMasterQuestSongSelectPanel.h"
#include "meta_ham/CampaignSongSelectPanel.h"
#include "meta_ham/ChallengeResultPanel.h"
#include "meta_ham/ChallengeSortMgr.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/ChooseModeProvider.h"
#include "meta_ham/ChooseProfilePanel.h"
#include "meta_ham/ContentLoadingPanel.h"
#include "meta_ham/CorrectIdentityPanel.h"
#include "meta_ham/CursorPanel.h"
#include "meta_ham/EventDialogPanel.h"
#include "meta_ham/FitnessGoalMgr.h"
#include "meta_ham/FitnessProvider.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamStarsDisplay.h"
#include "meta_ham/HamStorePanel.h"
#include "meta_ham/HelpBarPanel.h"
#include "meta_ham/KinectSharePanel.h"
#include "meta_ham/Leaderboards.h"
#include "meta_ham/LetterboxPanel.h"
#include "meta_ham/LoadingPanel.h"
#include "meta_ham/LockedContentPanel.h"
#include "meta_ham/MQSongSortMgr.h"
#include "meta_ham/MainMenuPanel.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/MetagameRank.h"
#include "meta_ham/MovieProvider.h"
#include "meta_ham/MultiUserGesturePanel.h"
#include "meta_ham/OptionsPanel.h"
#include "meta_ham/PassiveMessagesPanel.h"
#include "meta_ham/PlaylistSortMgr.h"
#include "meta_ham/PracticeChoosePanel.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SigninScreen.h"
#include "meta_ham/SingleUserCrewSelectPanel.h"
#include "meta_ham/SongSelectPanel.h"
#include "meta_ham/SongSelectPlaylistCustomizePanel.h"
#include "meta_ham/SongSelectPlaylistPanel.h"
#include "meta_ham/SongSortMgr.h"
#include "meta_ham/SongStatusMgr.h"
#include "meta_ham/TitleProvider.h"
#include "meta_ham/VoiceControlPanel.h"
#include "meta_ham/VoiceInputPanel.h"
#include "meta_ham/WeightInput.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/PostProc.h"
#include "stl/_vector.h"
#include "synth/Faders.h"
#include "synth/MetaMusic.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/HamSongMgr.h"
#include "obj/Dir.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "synth/Synth.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"
#include <cstring>

static SongDB *sSongDB; // DAT_821189a0
static HamMaster *sHamMaster; // DAT_8311899c

MetaPanel::MetaPanel() : unk44(0), unk4c(TheHamSongMgr), unkdc(false) {
    unke0 = new MetaMusicManager(SystemConfig("synth", "metamusic"));
    unke4 = new Campaign(SystemConfig("campaign"));
    unke8 = new HAQManager();
    unk4c.SetName("song_preview", ObjectDir::Main());
    SongSortMgr::Init(unk4c);
    ChallengeSortMgr::Init(unk4c);
    PlaylistSortMgr::Init(unk4c);
    MQSongSortMgr::Init(unk4c);
    FitnessCalorieSortMgr::Init(unk4c);
    unk38.reserve(3);
    for (int i = 3; i != 0; i--) {
        unk38.push_back(-1);
    }
    ThePlatformMgr.AddSink(this, "xmp_state_changed");
    sSongDB = new SongDB();
    sHamMaster = new HamMaster(sSongDB->SongData(), nullptr);
}

MetaPanel::~MetaPanel() {
    SongSortMgr::Terminate();
    ChallengeSortMgr::Terminate();
    FitnessCalorieSortMgr::Terminate();
    RELEASE(sSongDB);
    RELEASE(sHamMaster);
    RELEASE(TheMetaMusic);
}

Hmx::Object *MetaPanel::NewObject() { return new MetaPanel(); }

void MetaPanel::Init() {
    MetaInit();
    REGISTER_OBJ_FACTORY(AppLabel)
    REGISTER_OBJ_FACTORY(AppMiniLeaderboardDisplay)
    REGISTER_OBJ_FACTORY(HamPanel)
    REGISTER_OBJ_FACTORY(HamScreen)
    REGISTER_OBJ_FACTORY(CalibrationPanel)
    REGISTER_OBJ_FACTORY(CampaignDiffSelectPanel)
    REGISTER_OBJ_FACTORY(CampaignDiffProvider)
    REGISTER_OBJ_FACTORY(CampaignSongSelectPanel)
    REGISTER_OBJ_FACTORY(CampaignMasterQuestCrewSelectPanel)
    REGISTER_OBJ_FACTORY(CampaignMasterQuestSongSelectPanel)
    REGISTER_OBJ_FACTORY(ChallengeResultPanel)
    REGISTER_OBJ_FACTORY(ChooseProfilePanel)
    REGISTER_OBJ_FACTORY(CorrectIdentityPanel)
    REGISTER_OBJ_FACTORY(ContentLoadingPanel)
    REGISTER_OBJ_FACTORY(CreditsPanel)
    REGISTER_OBJ_FACTORY(EventDialogPanel)
    REGISTER_OBJ_FACTORY(FitnessProvider)
    REGISTER_OBJ_FACTORY(HamStorePanel)
    REGISTER_OBJ_FACTORY(LetterboxPanel)
    REGISTER_OBJ_FACTORY(BlacklightPanel)
    REGISTER_OBJ_FACTORY(HelpBarPanel)
    REGISTER_OBJ_FACTORY(LoadingPanel)
    REGISTER_OBJ_FACTORY(LockedContentPanel)
    REGISTER_OBJ_FACTORY(MainMenuPanel)
    REGISTER_OBJ_FACTORY(MetaPanel)
    MetaPerformer::Init();
    REGISTER_OBJ_FACTORY(MoviePanel)
    REGISTER_OBJ_FACTORY(MovieProvider)
    REGISTER_OBJ_FACTORY(TitleProvider)
    REGISTER_OBJ_FACTORY(ChooseModeProvider)
    REGISTER_OBJ_FACTORY(OptionsPanel)
    REGISTER_OBJ_FACTORY(SigninScreen)
    REGISTER_OBJ_FACTORY(SingleUserCrewSelectPanel)
    REGISTER_OBJ_FACTORY(SongSelectPanel)
    REGISTER_OBJ_FACTORY(SongSelectPlaylistCustomizePanel)
    REGISTER_OBJ_FACTORY(SongSelectPlaylistPanel)
    SongStatusMgr::Init();
    REGISTER_OBJ_FACTORY(TexLoadPanel)
    REGISTER_OBJ_FACTORY(Hmx::Object)
    TheMemcardMgr.Init();
    MetagameRank::Preinit();
    TheProfileMgr.Init();
    Leaderboards::Init();
    Challenges::Init();
    FitnessGoalMgr::Init();
    REGISTER_OBJ_FACTORY(HamStarsDisplay)
    REGISTER_OBJ_FACTORY(WeightInputPanel)
    REGISTER_OBJ_FACTORY(AppNavProvider)
    REGISTER_OBJ_FACTORY(MultiUserGesturePanel)
    REGISTER_OBJ_FACTORY(PassiveMessagesPanel)
    REGISTER_OBJ_FACTORY(CursorPanel)
    REGISTER_OBJ_FACTORY(PracticeChoosePanel)
    REGISTER_OBJ_FACTORY(VoiceControlPanel)
    REGISTER_OBJ_FACTORY(KinectSharePanel)
    REGISTER_OBJ_FACTORY(VoiceInputPanel)
    DataRegisterFunc("toggle_unlock_all", ToggleUnlockAll);
    DataRegisterFunc("toggle_motd_cheat", ToggleMotdCheat);
}

void MetaPanel::Load() {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            MILO_LOG("MetaPanel::Load suppressed, in game\n");
            return;
        }
    }
    UIPanel::Load();
    DataArray *sysConfig = SystemConfig("synth", "metamusic", "music");
    int loopIndex = PickLoopIndex(sysConfig->Size());
    DataArray *loopArray = sysConfig->Array(loopIndex);
    if (!TheMetaMusic) {
        TheMetaMusic = new MetaMusic(sHamMaster, "sfx/shell_fx.milo");
        TheMetaMusic->Load(0.0f, true, true);
        sHamMaster->Load(
            TheMetaMusic->SongInfo(), true, 0, false, (HamSongDataValidate)0, nullptr
        );
    }
    unk4c.Init();
    SystemConfig("sound", "banks");
    if (TheMetaMusic) {
        if (unkdc)
            TheMetaMusic->Mute();
        else
            TheMetaMusic->UnMute();
    }
}

void MetaPanel::Unload() {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            MILO_LOG("MetaPanel::Unload suppressed, in game\n");
            return;
        }
    }
    UIPanel::Unload();
    RndPostProc::Reset();
}

void MetaPanel::FinishLoad() {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            return;
        }
    }
    UIPanel::FinishLoad();
    TheMetaMusic->AddFader(TheSynth->Find<Fader>("background_music_level.fade", true));
}

bool MetaPanel::IsLoaded() const {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            return true;
        }
    }
    return UIPanel::IsLoaded() && TheMetaMusic->Loaded();
}

void MetaPanel::Poll() {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            return;
        }
    }
    UIPanel::Poll();
    unk4c.Poll();
    MILO_ASSERT(TheMetaMusic, 0x176);
    TheMetaMusic->Poll();
    float mstobeat = MsToBeat(sHamMaster->StreamMs());
    if (mstobeat > 0.0f) {
        TheTaskMgr.SetSecondsAndBeat(sHamMaster->StreamMs() * 0.001f, mstobeat, false);
    }
}

void MetaPanel::Enter() {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            return;
        }
    }
    UIPanel::Enter();
    sHamMaster->SetMaps();
    TheTaskMgr.SetAutoSecondsBeats(true);
}

void MetaPanel::Exit() {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            return;
        }
    }
    UIPanel::Exit();
    unk4c.Start(gNullStr, nullptr);
    TheMetaMusic->Stop();
    ThePlatformMgr.DisableXMP();
}

bool MetaPanel::Exiting() const {
    if (TheUI && TheUI->BottomScreen()) {
        if (strcmp(TheUI->BottomScreen()->Name(), "game_screen") == 0) {
            return false;
        }
    }
    if (mState != 2) {
        return UIPanel::Exiting();
    }
    bool busy = unk4c.IsWaitingToDelete() || unk4c.IsFadingOut()
        || TheMetaMusic->IsActive() || UIPanel::Exiting();
    if (!busy) {
        TheTaskMgr.SetAutoSecondsBeats(true);
    }
    return busy;
}

void MetaPanel::UnlockClassicOutfit(Symbol s) {
    std::vector<HamProfile *> profiles = TheProfileMgr.GetAll();
    FOREACH (it, profiles) {
        HamProfile *pProfile = *it;
        MILO_ASSERT(pProfile, 0x1ee);
        if (pProfile->HasValidSaveData()) {
            AccomplishmentProgress &pProgress = pProfile->AccessAccomplishmentProgress();
            static Symbol award_classic_outfits("award_classic_outfits");
            static Symbol award_classic_cheat("award_classic_cheat");
            pProgress.AddAward(s, award_classic_cheat);
        }
    }
}

DataNode MetaPanel::ToggleUnlockAll(DataArray *arr) {
    sUnlockAll = !sUnlockAll;
    return sUnlockAll;
}

DataNode MetaPanel::ToggleMotdCheat(DataArray *) {
    sMotdCheat = !sMotdCheat;
    return sMotdCheat;
}

DataNode MetaPanel::OnMsg(const XMPStateChangedMsg &msg) {
    bool success = msg.Success();
    unkdc = success;
    if (TheMetaMusic) {
        if (success) {
            TheMetaMusic->Mute();
        } else {
            TheMetaMusic->UnMute();
        }
    }
    return 0;
}

void MetaPanel::UnlockAll() {
    sUnlockAll = true;
    TheHamSongMgr.InitializePlaylists();
}

void MetaPanel::CycleVenuePreference() {
    Symbol venuePref = TheProfileMgr.GetVenuePreference();
    DataArray *pVenueArray = SystemConfig()->FindArray("venues", false);
    MILO_ASSERT(pVenueArray, 0x6f);
    static Symbol random_venue("random_venue");
    if (venuePref == random_venue) {
        DataArray *pVenueEntryArray = pVenueArray->Array(1);
        MILO_ASSERT(pVenueEntryArray, 0x76);
        venuePref = pVenueEntryArray->Sym(0);
    } else {
        short size = pVenueArray->Size();
        for (int i = 1; i < pVenueArray->Size(); i++) {
            DataArray *pVenueEntryArray = pVenueArray->Array(i);
            MILO_ASSERT(pVenueEntryArray, 0x80);
            Symbol entrySym = pVenueEntryArray->Sym(0);
            if (entrySym == venuePref) {
                if (i + 1 >= size) {
                    venuePref = random_venue;
                } else {
                    pVenueEntryArray = pVenueArray->Array(i + 1);
                    MILO_ASSERT(pVenueEntryArray, 0x8f);
                    venuePref = pVenueEntryArray->Sym(0);
                }
                break;
            }
        }
    }
    TheProfileMgr.SetVenuePreference(venuePref);
}

int MetaPanel::PickLoopIndex(int idx) {
    int size = unk38.size();
    int randomInt = RandomInt(1, idx);
    if (idx < size + 2) {
        return randomInt;
    }

    int i = 0;
    do {
        for (i = 0; i < size; i++) {
            if (randomInt == unk38[i]) {
                break;
            }
        }
        if (i == size)
            break;
        randomInt = RandomInt(1, idx);
    } while (true);

    unk38[unk44++] = randomInt;

    if (unk44 == size) {
        unk44 = 0;
    }

    return randomInt;
}

BEGIN_HANDLERS(MetaPanel)
    HANDLE_EXPR(meta_music, TheMetaMusic)
    HANDLE_ACTION(
        load_meta_music,
        sHamMaster->Load(
            TheMetaMusic->SongInfo(), true, 0, false, (HamSongDataValidate)0, 0
        )
    )
    HANDLE_ACTION(init_songpreview, unk4c.Init())
    HANDLE_ACTION(unlock_all, UnlockAll())
    HANDLE_ACTION(unlock_classic, UnlockClassicOutfit(_msg->Sym(2)))
    HANDLE_ACTION(cycle_venue_preference, CycleVenuePreference())
    HANDLE_EXPR(get_venue_preference, TheProfileMgr.GetVenuePreference())
    HANDLE_MESSAGE(XMPStateChangedMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
