#include "game/GamePanel.h"
#include "flow/PropertyEventProvider.h"
#include "game/Game.h"
#include "game/GameMode.h"
#include "game/PresenceMgr.h"
#include "game/SongDB.h"
#include "gesture/FitnessFilter.h"
#include "gesture/WaveToTurnOnLight.h"
#include "hamobj/CharFeedback.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/HamWardrobe.h"
#include "meta/HAQManager.h"
#include "meta/PreloadPanel.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "movie/TexMovie.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/Anim.h"
#include "rndobj/Overlay.h"
#include "rndobj/PostProc.h"
#include "rndobj/Rnd.h"
#include "synth/Sound.h"
#include "synth/StandardStream.h"
#include "synth/Synth.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "ui/UIScreen.h"
#include "utl/MBT.h"
#include "world/Dir.h"

GamePanel *TheGamePanel;
LoopVizCallback gLoopVizCallback;
LatencyCallback gGamePanelCallback;

GamePanel::GamePanel()
    : mGame(0), mTimeOverlay(RndOverlay::Find("time")),
      mLatencyOverlay(RndOverlay::Find("latency")),
      mFitnessOverlay(RndOverlay::Find("fitness")),
      mLoopVizOverlay(RndOverlay::Find("loop_viz")), unk7c(0), mState(), unk84(0),
      unk88("game_panel_load", 1), unkd8(0), unke8(0), unkec(-2), unkf0(0), unkf8(1),
      unkfc(new Timer()), unk100(1), unk101(0), unk104(0), unk108(0) {
    mFitnessFilters[0].SetPlayerIndex(0);
    mFitnessFilters[1].SetPlayerIndex(1);
    unkdc.resize(32);
    // set two globals/statics
    MILO_ASSERT(!TheGamePanel, 0x9E);
    TheGamePanel = this;
    SetType("none");
}

GamePanel::~GamePanel() {
    TheGamePanel = nullptr;
    RELEASE(unkfc);
}

BEGIN_HANDLERS(GamePanel)
    HANDLE_ACTION(set_start_paused, unk7c = _msg->Int(2))
    HANDLE_EXPR(in_intro, mState == kGameInIntro)
    HANDLE_EXPR(is_game_over, mState == kGameOver)
    HANDLE_EXPR(is_playing, mState == kGamePlaying)
    HANDLE_ACTION(start_game, StartGame())
    HANDLE(start_load_song, OnStartLoadSong)
    HANDLE(start_song_now, OnStartSongNow)
    HANDLE_ACTION(set_paused_except_sound, SetPausedHelper(_msg->Int(2), false))
    HANDLE_ACTION(cheat_pause, CheatPause(_msg->Int(2)))
    HANDLE_ACTION(clear_draw_glitch, ClearDrawGlitch())
    HANDLE_ACTION(reload_data, ReloadData())
    HANDLE_ACTION(win, SetGameOver(true))
    HANDLE_EXPR(is_past_stream_jump_point_of_no_return, IsPastStreamJumpPointOfNoReturn())
    HANDLE_ACTION(reset_limb_feedback, ResetLimbFeedback())
    HANDLE_ACTION(set_limb_feedback_visible, SetLimbFeedbackVisible(_msg->Int(2)))
    HANDLE(get_fitness_data, OnGetFitnessData)
    HANDLE_MESSAGE(EndGameMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

BEGIN_PROPSYNCS(GamePanel)
    {
        static Symbol _s("replay");
        if (sym == _s && (_op & kPropGet)) {
            return PropSync(unkd8, _val, _prop, _i + 1, _op);
        }
    }
    SYNC_SUPERCLASS(UIPanel)
END_PROPSYNCS

void GamePanel::SetTypeDef(DataArray *def) {
    TheHamProvider->SetProperty("game_stage", Symbol("intro"));
    static Message exit("exit_mode");
    Handle(exit, false);
    UIPanel::SetTypeDef(def);
}

void GamePanel::Load() {
    unkd8 = false;
    unk88.Start();
    CreateGame();
    UIPanel::Load();
}

void GamePanel::Enter() {
    TheTaskMgr.ClearTimelineTasks(kTaskSeconds);
    TheTaskMgr.ClearTimelineTasks(kTaskBeats);
    UIPanel::Enter();
    unk88.Stop();
    Reset();
    SetPaused(false);
    ThePresenceMgr.SetInGame(TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong()));
    //   DAT_83117414 = HamMaster::StreamMs(TheMaster);
    //   DAT_83117418 = 0.0;
}

void GamePanel::SetPaused(bool b1) { SetPausedHelper(b1, true); }

bool GamePanel::IsLoaded() const {
    if (!UIPanel::IsLoaded()) {
        return false;
    } else {
        return unk104 == 4;
    }
}

void GamePanel::Unload() {
    UIPanel::Unload();
    RELEASE(mGame);
    unk104 = 0;
    mPaused = false;
}

void GamePanel::FinishLoad() {
    UIPanel::FinishLoad();
    PreloadPanel::sCache->Clear();
    HAQManager::PrintSongInfo(TheGameData->GetSong(), TheSongDB->GetSongDurationMs());
}

FitnessFilter *GamePanel::GetFitnessFilter(int i1) {
    static Symbol is_in_party_mode("is_in_party_mode");
    static Symbol is_in_infinite_party_mode("is_in_infinite_party_mode");
    if (TheHamProvider->Property(is_in_party_mode)->Int() == 0
        && TheHamProvider->Property(is_in_infinite_party_mode)->Int() == 0) {
        HamPlayerData *pPlayerData = TheGameData->Player(i1);
        MILO_ASSERT(pPlayerData, 0x4A1);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayerData->PadNum());
        if (profile && profile->InFitnessMode()) {
            return &mFitnessFilters[i1];
        }
    }
    return nullptr;
}

void GamePanel::ResetJitter() {
    unke8 = 0;
    unkec = -2;
    unkf0 = 0;
}

void GamePanel::CreateGame() {
    RELEASE(mGame);
    mGame = new Game();
}

void GamePanel::StartGame() {
    AutoTimer::SetCollectStats(true, TheRnd.VerboseTimers());
    if (mGame->HasIntro()) {
        mGame->Start();
    }
    ThePresenceMgr.SetInGame(TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong()));
    mState = kGamePlaying;
}

void GamePanel::CheatPause(bool b1) {
    unk101 = b1;
    unk100 = false;
    SetPaused(unk101);
    unk100 = true;
}

void GamePanel::UpdateFitnessOverlay() {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        bool fitness = profile->InFitnessMode();
        float f1, f2, f3;
        profile->GetFitnessStats(f1, f2, f3);
        *mFitnessOverlay << MakeString(
            "Fitness %s: %.2f cal for this song, %.2f cal total, %s total time\n",
            fitness ? "on" : "off",
            f3,
            f2,
            FormatTimeMSH(f1 * 1000.0f)
        );
    }
}

void GamePanel::StartIntro() {
    mState = kGameInIntro;
    static Message pick_intro("pick_intro");
    HandleType(pick_intro);
    if (unk7c) {
        mGame->SetTimePaused(true);
    }
    mGame->StartIntro();
}

void GamePanel::SetGameOver(bool b1) {
    if (mState != kGameOver) {
        AutoTimer::SetCollectStats(false, TheRnd.VerboseTimers());
        EndGameResult r = mGame->GetResult(b1);
        static EndGameMsg msg((EndGameResult)3);
        msg[0] = r;
        Handle(msg, false);
    }
}

void GamePanel::ReloadData() {
    ObjectDir *hudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
    DataMacroWarning(false);
    DataArray *fileData = DataReadFile(SystemConfig()->File(), true);
    DataArray *objArr = fileData->FindArray("objects");
    ReloadObjectType(this, objArr);
    Hmx::Object *newObj = Hmx::Object::New<Hmx::Object>();
    newObj->SetType("point_value_chase");
    ReloadObjectType(newObj, objArr);
    delete newObj;
    FilePath proxy = hudPanel->ProxyFile();
    hudPanel->SetProxyFile(proxy, false);
    ReloadObjectType(hudPanel, objArr);
    fileData->Release();
    DataMacroWarning(true);
    static Message entermsg("enter");
    hudPanel->HandleType(entermsg);
    EndGameMsg msg((EndGameResult)0);
    Handle(msg, true);
}

void GamePanel::Reset() {
    for (int i = 0; i < 2; i++) {
        FitnessFilter *filt = GetFitnessFilter(i);
        if (filt) {
            filt->StartTracking();
        }
    }
    mGame->Reset();
    mState = (State)0;
    unk84 = 0;
    unkfc->Reset();
    unk101 = false;
    WorldDir *dir = TheHamDirector->GetVenueWorld();
    for (ObjDirItr<TexMovie> it(dir, true); it != nullptr; ++it) {
        it->Reset();
    }
    mGame->Restart(true);
    static Message resetMsg("reset");
    Export(resetMsg, true);
}

void GamePanel::SetSoundEventReceiver() {
    if (!unk108) {
        ObjectDir *hudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
        ObjectDir *soundBank = hudPanel->Find<ObjectDir>("sound_bank", false);
        if (soundBank) {
            for (ObjDirItr<Sound> it(soundBank, true); it != nullptr; ++it) {
                if (it->NumMarkers() > 0) {
                    it->SetSoundEventReceiver(this);
                }
            }
            unk108 = true;
        }
    }
}

void GamePanel::SetPausedHelper(bool b1, bool b2) {
    for (int i = 0; i < 2; i++) {
        FitnessFilter *filt = GetFitnessFilter(i);
        if (filt) {
            filt->SetPaused(b1);
        }
    }
    if (GetState() != kUp) {
        MILO_NOTIFY("trying to pause while not up");
        return;
    }
    while (TheSynth->HasPendingVoices()) {
        TheSynth->Poll();
    }
    if (!b1 && unk101) {
        return;
    }
    if (b1 == mPaused) {
        return;
    }
    mPaused = b1;
    if (unkfc->Running()) {
        if (!b1) {
            MILO_NOTIFY(
                "Trying to unpause while the count in is active; should not be possible!"
            );
        }
        unkfc->Reset();
    } else {
        if (unk100 && mState == 2 && !b1) {
            if (TheGameMode->Property("pause_count_in")->Int() != 0) {
                unkfc->Start();
            } else {
                mGame->SetGamePaused(b1, mState == 0 || mState == 1, b2);
                WorldDir *dir = TheHamDirector->GetVenueWorld();
                for (ObjDirItr<TexMovie> it(dir, true); it != nullptr; ++it) {
                    if (it->IsOpen()) {
                        it->SetPaused(b1);
                    }
                }
                TheWaveToTurnOnLight->SetPaused(b1);
                if (!b1) {
                    while (!FileDiscSpinUp())
                        ;
                }
            }
        }
    }
    UpdateNowBar();
}

void GamePanel::ResetLimbFeedback() {
    WorldDir *dir = TheHamDirector->GetVenueWorld();
    for (ObjDirItr<CharFeedback> it(dir, true); it != nullptr; ++it) {
        it->ResetErrors();
    }
}

void GamePanel::SetLimbFeedbackVisible(bool visible) {
    WorldDir *dir = TheHamDirector->GetVenueWorld();
    for (ObjDirItr<CharFeedback> it(dir, true); it != nullptr; ++it) {
        it->SetShowing(visible);
    }
}

DataNode GamePanel::OnStartLoadSong(DataArray *a) {
    Symbol song = a->ForceSym(2);
    QuickplayPerformer *qp = dynamic_cast<QuickplayPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(qp, 0x446);
    qp->SetSong(song);
    CreateGame();
    return 0;
}

DataNode GamePanel::OnStartSongNow(DataArray *a) {
    Reset();
    StartIntro();
    return 0;
}

DataNode GamePanel::OnGetFitnessData(const DataArray *a) {
    int index = a->Int(2);
    FitnessFilter *filter = GetFitnessFilter(index);
    if (!filter) {
        return 0;
    } else if (a->Size() > 3) {
        float f1, f2;
        filter->GetFitnessData(f1, f2);
        bool b3 = a->Int(3);
        if (b3) {
            HamPlayerData *pPlayerData = TheGameData->Player(index);
            MILO_ASSERT(pPlayerData, 0x4BC);
            HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayerData->PadNum());
            f1 += profile->FitnessCalories();
            f2 += profile->FitnessTime();
        }
        if (a->Size() > 4) {
            *a->Node(4).Var() = f1;
        }
        if (a->Size() > 5) {
            *a->Node(5).Var() = f2;
        }
    }
    return 1;
}

DataNode GamePanel::OnMsg(const EndGameMsg &msg) {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayerData = TheGameData->Player(i);
        MILO_ASSERT(pPlayerData, 0x3F5);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayerData->PadNum());
        FitnessFilter *filter = GetFitnessFilter(i);
        float f1, f2;
        if (filter && filter->GetFitnessDataAndReset(f1, f2)) {
            profile->SetFitnessStats(i, f1, f2);
        }
    }
    EndGameResult r = msg.Result();
    if (r != 1 && r != 2) {
        MetaPerformer::Current()->HandleGameplayEnded(r);
    }
    if (mGame) {
        mGame->ClearState();
    }
    if (msg.Result() == 0) {
        static Symbol game_restart("game_restart");
        static DataArrayPtr restart(game_restart);
        restart->Execute();
    } else {
        mState = kGameOver;
        unk84 = msg.Result();
        switch (unk84) {
        case 1:
            Export(Message("game_won"), true);
            break;
        case 2:
            Export(Message("game_won_finale"), true);
            break;
        case 3:
            Export(Message("game_over"), true);
            break;
        default:
            MILO_NOTIFY("bad game over state");
            break;
        }
    }
    return 1;
}

bool GamePanel::IsPastStreamJumpPointOfNoReturn() {
    StandardStream *stream =
        dynamic_cast<StandardStream *>(TheMaster->GetAudio()->GetSongStream());
    return stream->IsPastStreamJumpPointOfNoReturn();
}

void GamePanel::PollForLoading() {
    unk104 = 0;
    UIPanel::PollForLoading();
    if (UIPanel::IsLoaded()) {
        unk104 = 1;
        UIPanel *worldPanel = ObjectDir::Main()->Find<UIPanel>("world_panel");
        if (TheUI->TransitionScreen()
            && TheUI->TransitionScreen()->HasPanel(worldPanel)) {
            if (!TheHamDirector) {
                return;
            }
            if (!TheHamDirector->IsWorldLoaded()) {
                return;
            }
        }
        unk104 = 2;
        const DataNode *prop = TheGameMode->Property("load_chars");
        if (prop->Int() != 0 && !TheHamWardrobe->AllCharsLoaded()) {
            return;
        }
        unk104 = 3;
        if (mGame->IsReady()) {
            unk104 = 4;
        }
    }
}

void GamePanel::Exit() {
    TheTaskMgr.ClearTimelineTasks(kTaskSeconds);
    TheTaskMgr.ClearTimelineTasks(kTaskBeats);
    ThePresenceMgr.SetNotInGame();
    UIPanel::Exit();
    unkd8 = true;
    for (int i = 0; i < 2; i++) {
        FitnessFilter *filter = GetFitnessFilter(i);
        if (filter) {
            filter->StopTracking();
        }
    }
    RndAnimatable *beatRepeatAnim = TheSynth->Find<RndAnimatable>("beat_repeat.anim");
    if (beatRepeatAnim) {
        beatRepeatAnim->SetFrame(4.0f, 1.0f);
    }
    unk108 = false;
}

void GamePanel::ClearDrawGlitch() {
    UIScreen *gameScreen = ObjectDir::Main()->Find<UIScreen>("game_screen");
    gameScreen->SetShowing(false);
    RndPostProc::Reset();
    for (int i = 0; i < 2; i++) {
        TheRnd.BeginDrawing();
        TheRnd.EndDrawing();
    }
}
