#include "game/GamePanel.h"
#include "GamePanel.h"
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
#include "hamobj/MoveDir.h"
#include "math/Utl.h"
#include "meta/HAQManager.h"
#include "meta/PreloadPanel.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/UIEventMgr.h"
#include "meta_ham/Utl.h"
#include "movie/TexMovie.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Joypad.h"
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
#include "utl/FilePath.h"
#include "utl/MBT.h"
#include "utl/MakeString.h"
#include "utl/SongPos.h"
#include "utl/TimeConversion.h"
#include "world/Dir.h"
#include <cmath>

GamePanel *TheGamePanel = nullptr;
LoopVizCallback gLoopVizCallback;
LatencyCallback gGamePanelCallback;
static float sLastStreamMs = 0;
static float sDeltaSeconds = 0;

float LatencyCallback::UpdateOverlay(RndOverlay *o, float f2) {
    Hmx::Color color = mWhite ? Hmx::Color(1, 1, 1) : Hmx::Color(0, 0, 0);
    TheRnd.DrawRectScreen(
        Hmx::Rect(0, 0.875f, 0.125f, 0.125f), color, nullptr, nullptr, nullptr
    );
    TheRnd.DrawRectScreen(
        Hmx::Rect(0, 0.125f, 0.125f, 0.125f), color, nullptr, nullptr, nullptr
    );
    return f2;
}

#pragma region LoopVizCallback

LoopVizCallback::LoopVizCallback()
    : mSongMeter(0.1f, 0.12f, 0.8f, 0.03f, Hmx::Color(0.1f, 0.1f, 0.1f)),
      mLoopMeter(0.25f, 0.19f, 0.5f, 0.03f, Hmx::Color(0, 0, 0.8f)), mLoopStart(0),
      mLoopEnd(0), mOldLoopStart(0), mOldLoopEnd(0), mHighlightLoopStart(0),
      mHighlightLoopEnd(0) {}

void LoopVizCallback::DrawHashMarks(
    float f1, float f2, float f3, int i4, int i5, bool b6
) {
    if (i4 <= 3000) {
        for (int i = 0; i <= i4; i++) {
            int u4 = i + i5;
            float fvar1 = ((float)i / (float)i4) * f3 + f2;
            if (u4 % 4 == 0) {
                TheRnd.DrawRectScreen(
                    Hmx::Rect(fvar1, f1 - 0.01f, 0.001f, 0.01f),
                    Hmx::Color(1, 1, 0),
                    nullptr,
                    nullptr,
                    nullptr
                );
            } else if (b6) {
                TheRnd.DrawRectScreen(
                    Hmx::Rect(fvar1, f1 - 0.003f, 0.001f, 0.003f),
                    Hmx::Color(1, 1, 0),
                    nullptr,
                    nullptr,
                    nullptr
                );
            }
        }
    }
}

float LoopVizCallback::UpdateOverlay(RndOverlay *o, float y) {
    if (TheMaster && TheMaster->GetAudio() && TheMaster->GetAudio()->GetSongStream()) {
        mHighlightLoopStart -= TheTaskMgr.DeltaSeconds();
        mHighlightLoopEnd -= TheTaskMgr.DeltaSeconds();
        TheRnd.DrawRectScreen(
            Hmx::Rect(0.05f, 0.1f, 0.9f, 0.2f),
            Hmx::Color(0, 0, 0, 0.6f),
            nullptr,
            nullptr,
            nullptr
        );
        int loopStart, loopEnd;
        TheMaster->GetAudio()->GetCurrLoopBeats(loopStart, loopEnd);
        if (loopStart != mLoopStart) {
            mHighlightLoopStart = 3.0f;
            mLoopStart = loopStart;
        }
        if (loopEnd != mLoopEnd) {
            mHighlightLoopEnd = 3.0f;
            mLoopEnd = loopEnd;
        }
        float beat = MsToBeat(TheMaster->StreamMs());
        float f14 = (beat - (float)loopStart) / (float)(loopEnd - loopStart);
        StandardStream *stream =
            dynamic_cast<StandardStream *>(TheMaster->GetAudio()->GetSongStream());
        float beatAhead = MsToBeat(stream->GetBufferAheadTime());
        float f10 = (beatAhead - beat) / (float)(loopEnd - loopStart);
        static Symbol end("end");
        int eventBeat = TheMaster->EventBeat(end);
        float f16 = (float)loopStart / (float)eventBeat;
        float f12 = (float)loopEnd / (float)eventBeat;
        mSongMeter.Draw();
        Hmx::Color color(0.8f, 0.8f, 0.8f);
        mSongMeter.DrawBar(f16, f12 - f16, color);
        mSongMeter.DrawBar(f16, (f12 - f16) * f14, color);
        // more...
        if (stream->IsPastStreamJumpPointOfNoReturn()) {
        }
    }
    return y;
}

#pragma endregion
#pragma region GamePanel

GamePanel::GamePanel()
    : mGame(0), mNowBar(RndOverlay::Find("time")), mLatency(RndOverlay::Find("latency")),
      mFitness(RndOverlay::Find("fitness")), mLoopViz(RndOverlay::Find("loop_viz")),
      mStartPaused(0), mState(), mResult(kRestart), mLoadProf("game_panel_load", 1),
      mReplay(0), mJitterIndex(0), mJitterWindow(-2), mLastAverage(0), mDriveTime(true),
      mPauseCountInTimer(new Timer()), mShouldCountIn(true), mCheatPaused(false),
      mLoadingState(kLoadingState_NotReady), mbSoundEventReceiverSet(false) {
    mFitnessFilter[0].SetPlayerIndex(0);
    mFitnessFilter[1].SetPlayerIndex(1);
    mJitter.resize(32);
    sLastStreamMs = sDeltaSeconds = 0;
    MILO_ASSERT(!TheGamePanel, 0x9E);
    TheGamePanel = this;
    SetType("none");
}

GamePanel::~GamePanel() {
    TheGamePanel = nullptr;
    RELEASE(mPauseCountInTimer);
}

BEGIN_HANDLERS(GamePanel)
    HANDLE_ACTION(set_start_paused, mStartPaused = _msg->Int(2))
    HANDLE_EXPR(in_intro, mState == kGameNeedStart)
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
            return PropSync(mReplay, _val, _prop, _i + 1, _op);
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
    mReplay = false;
    mLoadProf.Start();
    CreateGame();
    UIPanel::Load();
}

void GamePanel::Enter() {
    TheTaskMgr.ClearTimelineTasks(kTaskSeconds);
    TheTaskMgr.ClearTimelineTasks(kTaskBeats);
    UIPanel::Enter();
    mLoadProf.Stop();
    Reset();
    SetPaused(false);
    ThePresenceMgr.SetInGame(TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong()));
    sLastStreamMs = TheMaster->StreamMs();
    sDeltaSeconds = 0;
}

void GamePanel::Exit() {
    TheTaskMgr.ClearTimelineTasks(kTaskSeconds);
    TheTaskMgr.ClearTimelineTasks(kTaskBeats);
    ThePresenceMgr.SetNotInGame();
    UIPanel::Exit();
    mReplay = true;
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
    mbSoundEventReceiverSet = false;
}

void GamePanel::Poll() {
    START_AUTO_TIMER("game_poll");
    SetSoundEventReceiver();
    if (!IsLoaded()) {
        return;
    } else {
        if (mPauseCountInTimer->SplitMs() >= 100.0f) {
            while (!FileDiscSpinUp()) {
                MILO_LOG("Spinning up disc took longer than count in timer\n");
            }
            MILO_ASSERT(mState == kGamePlaying, 0x1C5);
            mGame->SetGamePaused(false, true, true);
            mPauseCountInTimer->Reset();
        } else if (mPauseCountInTimer->Running()) {
            FileDiscSpinUp();
        }
        if (!mGame->Paused() && TheUIEventMgr->HasActiveDialogEvent()) {
            static Message pauseGameMsg("pause_game");
            Handle(pauseGameMsg, true);
        }
        UIPanel::Poll();
        if (mState == kGameNeedIntro) {
            StartIntro();
        }
        if (!mPauseCountInTimer->Running()) {
            mGame->Poll();
        }
        if (mState == kGameNeedStart
            && TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo) > -0.025f
            && !TheHamDirector->GetGameStartHold()) {
            StartGame();
        }
        for (int i = 0; i < 2; i++) {
            FitnessFilter *filt = GetFitnessFilter(i);
            if (filt) {
                filt->Poll();
            }
        }
        MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves", false);
        static Message isTrackingScoreMsg("is_tracking_score");
        DataNode handled = Handle(isTrackingScoreMsg, false);
        if (moves && handled.Type() != kDataUnhandled) {
            moves->SetFiltersEnabled(handled.Int());
        }
        if (mNowBar->Showing()) {
            UpdateNowBar();
        }
        UpdateLatency();
        if (mFitness->Showing()) {
            UpdateFitnessOverlay();
        }
        if (mLoopViz->Showing()) {
            mLoopViz->SetCallback(&gLoopVizCallback);
        }
        if (TheMaster) {
            float ms = TheMaster->StreamMs();
            sDeltaSeconds = (ms - sLastStreamMs) / 1000.0f;
            sLastStreamMs = ms;
        } else {
            sDeltaSeconds = 0;
        }
    }
}

void GamePanel::SetPaused(bool b1) { SetPausedHelper(b1, true); }

bool GamePanel::IsLoaded() const {
    if (!UIPanel::IsLoaded()) {
        return false;
    } else {
        return mLoadingState == kLoadingState_Ready;
    }
}

void GamePanel::Unload() {
    UIPanel::Unload();
    RELEASE(mGame);
    mLoadingState = kLoadingState_NotReady;
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
            return &mFitnessFilter[i1];
        }
    }
    return nullptr;
}

void GamePanel::ResetJitter() {
    mJitterIndex = 0;
    mJitterWindow = -2;
    mLastAverage = 0;
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
    mCheatPaused = b1;
    mShouldCountIn = false;
    SetPaused(mCheatPaused);
    mShouldCountIn = true;
}

void GamePanel::UpdateFitnessOverlay() {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        bool fitness = profile->InFitnessMode();
        float f1, f2, f3;
        profile->GetFitnessStats(f1, f2, f3);
        *mFitness << MakeString(
            "Fitness %s: %.2f cal for this song, %.2f cal total, %s total time\n",
            fitness ? "on" : "off",
            f3,
            f2,
            FormatTimeMSH(f1 * 1000.0f)
        );
    }
}

void GamePanel::StartIntro() {
    mState = kGameNeedStart;
    static Message pick_intro("pick_intro");
    HandleType(pick_intro);
    if (mStartPaused) {
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
    DataArray *objectsConfig = fileData->FindArray("objects");
    ReloadObjectType(this, objectsConfig);
    Hmx::Object *newObj = Hmx::Object::New<Hmx::Object>();

    newObj->SetType("point_value_chase");
    ReloadObjectType(newObj, objectsConfig);
    delete newObj;

    FilePath path = hudPanel->ProxyFile();
    hudPanel->SetProxyFile(path, false);
    ReloadObjectType(hudPanel, objectsConfig);
    fileData->Release();

    DataMacroWarning(true);

    static Message entermsg("enter");
    hudPanel->HandleType(entermsg);

    EndGameMsg endGame(kRestart);
    Handle(endGame, true);
}

void GamePanel::Reset() {
    for (int i = 0; i < 2; i++) {
        FitnessFilter *filt = GetFitnessFilter(i);
        if (filt) {
            filt->StartTracking();
        }
    }
    mGame->Reset();
    mState = kGameNeedIntro;
    mResult = kRestart;
    mPauseCountInTimer->Reset();
    mCheatPaused = false;
    WorldDir *dir = TheHamDirector->GetVenueWorld();
    for (ObjDirItr<TexMovie> it(dir, true); it != nullptr; ++it) {
        it->Reset();
    }
    mGame->Restart(true);
    static Message resetMsg("reset");
    Export(resetMsg, true);
}

void GamePanel::SetSoundEventReceiver() {
    if (!mbSoundEventReceiverSet) {
        ObjectDir *hudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
        ObjectDir *soundBank = hudPanel->Find<ObjectDir>("sound_bank", false);
        if (soundBank) {
            for (ObjDirItr<Sound> it(soundBank, true); it != nullptr; ++it) {
                if (it->NumMarkers() > 0) {
                    it->SetEventReceiver(this);
                }
            }
            mbSoundEventReceiverSet = true;
        }
    }
}

void GamePanel::SetPausedHelper(bool paused, bool pauseSFX) {
    for (int i = 0; i < 2; i++) {
        FitnessFilter *filt = GetFitnessFilter(i);
        if (filt) {
            filt->SetPaused(paused);
        }
    }
    if (GetState() != kUp) {
        MILO_NOTIFY("trying to pause while not up");
        return;
    }
    while (TheSynth->HasPendingVoices()) {
        TheSynth->Poll();
    }
    if (!paused && mCheatPaused) {
        return;
    }
    if (paused == mPaused) {
        return;
    }
    mPaused = paused;
    if (mPauseCountInTimer->Running()) {
        if (!paused) {
            MILO_NOTIFY(
                "Trying to unpause while the count in is active; should not be possible!"
            );
        }
        mPauseCountInTimer->Reset();
    } else {
        if (mShouldCountIn && mState == kGamePlaying && !paused
            && TheGameMode->Property("pause_count_in")->Int() != 0) {
            mPauseCountInTimer->Start();
        } else {
            mGame->SetGamePaused(paused, mState >= kGamePlaying, pauseSFX);
            WorldDir *dir = TheHamDirector->GetVenueWorld();
            for (ObjDirItr<TexMovie> it(dir, true); it != nullptr; ++it) {
                if (it->IsOpen()) {
                    it->SetPaused(paused);
                }
            }
            TheWaveToTurnOnLight->SetPaused(paused);
            while (!paused) {
                paused = FileDiscSpinUp();
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
            f1 += profile->GetFitnessCalories();
            f2 += profile->GetFitnessTime();
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
        mResult = msg.Result();
        switch (mResult) {
        case kWonFinale: {
            Export(Message("game_won_finale"), true);
        }
        case kWon: {
            Export(Message("game_won"), true);
        }
        case kQuit: {
            Export(Message("game_over"), true);
            break;
        }
        default:
            MILO_NOTIFY("bad game over state");
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
    mLoadingState = kLoadingState_NotReady;
    UIPanel::PollForLoading();
    if (UIPanel::IsLoaded()) {
        mLoadingState = kLoadingState_UILoaded;
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
        mLoadingState = kLoadingState_WorldLoaded;
        const DataNode *prop = TheGameMode->Property("load_chars");
        if (prop->Int() != 0 && !TheHamWardrobe->AllCharsLoaded()) {
            return;
        }
        mLoadingState = kLoadingState_CharsLoaded;
        if (mGame->IsReady()) {
            mLoadingState = kLoadingState_Ready;
        }
    }
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

void GamePanel::UpdateNowBar() {
    MILO_ASSERT(mGame, 0x23d);
    float sec = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
    float songDuration = TheSongDB->GetSongDurationMs();
    char sec_left_prefix = '-';
    float secLeft = (songDuration / 1000.0f) - sec;
    float percentIntoSong;
    if (secLeft < 0.0f) {
        secLeft = -secLeft;
        sec_left_prefix = '+';
    }
    int totalTick = TheTaskMgr.GetSongPos().GetTotalTick();

    if (0.0f < songDuration) {
        percentIntoSong = 100.0f;
        float percent = (sec * 100.0f) / (songDuration * 0.001f);
        if (percent <= 100.0f) {
            percentIntoSong = percent;
        }
    }
    const SongPos &songPos = TheTaskMgr.GetSongPos();
    *mNowBar << MakeString(
        "MBT %d:%d:%03d [%s %c%s %4.1f%%] (%.2fsec %dtk)\n",
        songPos.GetMeasure() + 1,
        songPos.GetBeat() + 1,
        songPos.GetTick(),
        FormatTimeMSH(sec * 1000.0f),
        sec_left_prefix,
        FormatTimeMSH(secLeft * 1000.0f),
        percentIntoSong,
        sec,
        totalTick
    );
}

void GamePanel::UpdateLatency() {
    static Hmx::Object *beep;
    static float frames[2] = { 0, 0 };
    static int frame = 0;
    static int count = 0;
    static bool wasPressed = false;
    static DataNode &latency_test = DataVariable("latency_test");
    static DataNode &pad_button = DataVariable("pad_button");

    if (latency_test.Int() == 0) {
        mLatency->SetShowing(false);
    } else {
        bool isWhite = false;
        bool justPressed = false;
        int player = latency_test.Int() - 2;
        if (player == -1) {
            float floorBeat = floor(TheTaskMgr.Beat() * 2.0f);
            float floorDeltaBeat =
                floor((TheTaskMgr.Beat() - TheTaskMgr.DeltaBeat()) * 2.0f);
            if (floorBeat != floorDeltaBeat) {
                count = 3;
            }
            if (0 < count) {
                count--;
                isWhite = true;
            }
        } else {
            JoypadData *data = JoypadGetPadData(player);
            if (data) {
                bool pressed = ((1 << pad_button.Int()) & data->mButtons) != 0;
                // i got no idea what happens in the rest of this if statement
                justPressed = pressed && !wasPressed;
            }
        }
        gGamePanelCallback.mWhite = isWhite;
        if (justPressed) {
            if (!beep) {
                ObjectDir *load =
                    DirLoader::LoadObjects("test/latency.milo", nullptr, nullptr);
                beep = load->Find<Hmx::Object>("beep.cue");
            }
            static Message playMsg("play");
            beep->Handle(playMsg);
        }
        int idx = frame;
        frame = 1 - frame;
        frames[idx] = TheRnd.DrawMs();
        mLatency->SetShowing(false);
        mLatency->Clear();
        *mLatency << MakeString(
            "Joy %d Beat %.3f\nms %.2f last %.2f",
            player,
            TheTaskMgr.Beat(),
            frames[0],
            frames[1]
        );
        mLatency->SetCallback(&gGamePanelCallback);
    }
}

float GamePanel::DeJitter(float ms) {
    float f = 1.0e+30f;
    static DataNode &no_jitter = DataVariable("no_jitter");
    if (no_jitter.Int() == 0 && 8 < mJitterWindow) {
        int endPos = (mJitterIndex - 1) & 0x1f;
        int x = (endPos - mJitterWindow) & 0x1f;
        float averageJitter = (mJitter[endPos] - mJitter[x]) / (float)mJitterWindow;
        if (mLastAverage == 0) {
            mLastAverage = averageJitter;
        }

        // yeah idk whats happening after here yall
        mLastAverage = (averageJitter - mLastAverage) * 0.1f + mLastAverage;

        f = mLastMs + mLastAverage;
        f = Clamp(ms - 16.0f, ms + 16.0f, f);
        if (f < mLastMs) {
            f = mLastMs;
        }
    }

    mJitter[mJitterIndex] = ms;
    if (f != 1.0e+30f) {
        ms = f;
    }

    mJitterIndex = (mJitterIndex + 1) & 0x1f;
    if (mJitterWindow < 30) {
        mJitterWindow++;
    }
    mLastMs = ms;
    return ms;
}

#pragma endregion
