#include "game/Game.h"
#include "Game.h"
#include "SongDB.h"
#include "char/FileMerger.h"
#include "flow/PropertyEventProvider.h"
#include "game/BustAMovePanel.h"
#include "game/GameMode.h"
#include "game/GamePanel.h"
#include "game/HamUser.h"
#include "game/LiveInput.h"
#include "game/Shuttle.h"
#include "game/SongDB.h"
#include "game/SongSequence.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonClip.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/CharFeedback.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/HamSongData.h"
#include "hamobj/MoveDir.h"
#include "hamobj/MoveMgr.h"
#include "hamobj/SuperEasyRemixer.h"
#include "macros.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/Overshell.h"
#include "meta_ham/ProfileMgr.h"
#include "midi/MidiParserMgr.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "synth/Faders.h"
#include "synth/Sequence.h"
#include "synth/Synth.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/MultiTempoTempoMap.h"
#include "utl/SongInfoCopy.h"
#include "utl/SongPos.h"
#include "utl/Symbol.h"
#include "utl/TempoMap.h"
#include "utl/TimeConversion.h"
#include "world/Dir.h"

Game *TheGame;
static bool sMoveOverlayToggle;
std::vector<Symbol> sAutoplayStates;

Game::Game()
    : mSongDB(new SongDB()), mSongInfo(0), mGameInput(0), unk58(0), unk5c(false),
      unk5d(false), unk5e(true), unk5f(false), unk60(false), unk64(0), unk68(false),
      unk6c(1), unk70(false), unk71(false), mOvershell(0), mMoveDir(this), unk90(0),
      mShuttle(new Shuttle()), unka4(0), unka8(0), unkac(0) {
    if (TheSongDB) {
        RELEASE(TheSongDB);
    }
    TheSongDB = mSongDB;
    TheGame = this;
    unka0 = 0;
    SetName("game", ObjectDir::Main());
    MidiParserMgr *lol = new MidiParserMgr(nullptr, "biteme");
    TheMaster = new HamMaster(mSongDB->SongData(), TheMidiParserMgr);
    mMaster = TheMaster;
    TheMaster->SetName("master", ObjectDir::Main());
    TheMaster->GetAudio()->SetName("audio", ObjectDir::Main());
    SetBackgroundVolume(TheProfileMgr.GetMusicVolumeDb());
    SetForegroundVolume(TheProfileMgr.GetMusicVolumeDb());
    mMaster->GetAudio()->SetStereo(!TheProfileMgr.Mono());
    LoadSong();
    unk72 = false;
    unk73 = false;
    unk74 = true;
    SkeletonUpdateHandle h = SkeletonUpdate::InstanceHandle();
    h.AddCallback(this);
}

Game::~Game() {
    SkeletonUpdateHandle h = SkeletonUpdate::InstanceHandle();
    h.RemoveCallback(this);
    SetHamMove(0, nullptr, false);
    TheSongSequence.Clear();
    RELEASE(mGameInput);
    RELEASE(mShuttle);
    TheGame = nullptr;
    TheSongDB = nullptr;
    TheMaster = nullptr;
    RELEASE(mMaster);
    RELEASE(mSongDB);
    RELEASE(mSongInfo);
    RELEASE(TheMidiParserMgr);
    RELEASE(mOvershell);
}

BEGIN_HANDLERS(Game)
    HANDLE_ACTION(start, Start())
    HANDLE_EXPR(get_song_ms, mMaster->GetAudio()->GetTime())
    HANDLE_ACTION(set_music_volume, SetMusicVolume(_msg->Float(2)))
    HANDLE_ACTION(
        set_paused,
        SetGamePaused(_msg->Int(2), true, _msg->Size() > 3 ? _msg->Int(3) : false)
    )
    HANDLE_EXPR(get_paused, unk5e)
    HANDLE_ACTION(never_allow_input, unk70 = _msg->Int(2))
    HANDLE_ACTION(set_time_paused, SetTimePaused(_msg->Int(2)))
    HANDLE_EXPR(time_paused, unk5f)
    HANDLE(set_shuttle, OnSetShuttle)
    HANDLE_EXPR(shuttle_active, mShuttle->IsActive())
    HANDLE_ACTION(jump, Jump(_msg->Float(2), true))
    HANDLE_ACTION(set_intro_real_time, SetIntroRealTime(_msg->Float(2)))
    HANDLE_ACTION(set_realtime, SetRealTime(_msg->Int(2)))
    HANDLE_EXPR(get_realtime, unk60)
    HANDLE_ACTION(is_active_user, _msg->Obj<HamUser>(2))
    HANDLE_EXPR(
        ms_per_beat, TheTempoMap ? TheTempoMap->GetTempo(TheTaskMgr.CurrentTick()) : 0.0f
    )
    HANDLE_EXPR(get_result, GetResult(true))
    HANDLE_EXPR(get_result_for_user, (_msg->Obj<HamUser>(2), GetResult(true)))
    HANDLE_EXPR(is_waiting, IsWaiting())
    HANDLE_ACTION(reset_audio, ResetAudio())
    HANDLE_ACTION(set_loop, SetLoop(_msg->Int(2)))
    HANDLE_ACTION(
        force_serial_sequences, RandomGroupSeq::ForceSerialSequences(_msg->Int(2))
    )
    HANDLE_EXPR(using_serial_sequences, RandomGroupSeq::UsingSerialSequences())
    HANDLE(reset_detection, OnResetDetection)
    HANDLE_ACTION(set_cur_move, SetHamMove(_msg->Int(2), _msg->Obj<HamMove>(3), true))
    HANDLE_EXPR(get_cur_move, mMoveDir->CurrentMove(_msg->Int(2)))
    HANDLE_ACTION(reload_song, ReloadSong())
    HANDLE_EXPR(is_ready, IsReady())
    HANDLE_ACTION(
        load_new_song, LoadNewSong(_msg->Sym(2), _msg->Size() > 2 ? _msg->Sym(3) : 0)
    )
    HANDLE_ACTION(load_new_song_audio, LoadNewSongAudio(_msg->Sym(2)))
    HANDLE_ACTION(load_new_song_moves, LoadNewSongMoves(_msg->Sym(2), true))
    HANDLE_ACTION(load_new_venue, LoadNewVenue(_msg->Sym(2)))
    HANDLE_ACTION(swap_move_record, SwapMoveRecord())
    HANDLE_ACTION(flush_move_record, FlushMoveRecord())
    HANDLE_EXPR(is_song_default_player_playing, IsSongDefaultPlayerPlaying())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Game)
    SYNC_PROP_SET(music_speed, unk6c, SetMusicSpeed(_val.Float()))
END_PROPSYNCS

void Game::PostUpdate(const SkeletonUpdateData *data) {
    if (data) {
        if (TheTaskMgr.Seconds(TaskMgr::kRealTime) >= 0 && !TheGamePanel->IsGameOver()) {
            if (!unk5e) {
                static Symbol practice("practice");
                static Symbol gameplay_mode("gameplay_mode");
                if (TheGameMode->Property(gameplay_mode)->Sym() != practice) {
                    mOvershell->Poll((const Skeleton *(&)[6])data->unk4);
                }
                CheckForSkeletonLoss((const Skeleton *(&)[6])data->unk4);
            }
        }
    }
}

void Game::Start() {
    mHasIntro = false;
    unka4 = unka4 == 3 ? 4 : 1;
}

bool Game::HasIntro() { return mHasIntro; }

void Game::ClearState() {
    if (mMoveDir) {
        mMoveDir->FinishGameRecord();
    }
}

void Game::PostWaitRestart() {
    SetMusicSpeed(1.0f);
    if (!mHasIntro)
        PostWaitStart();
}

void Game::LoadNewVenue(Symbol newVenue) {
    static Symbol venue("venue");
    TheGameData->SetVenue(newVenue);
    SetPaused(true, true);
    UIPanel *gamePanel = ObjectDir::Main()->Find<UIPanel>("game_panel", false);
    gamePanel->SetPaused(true);
    TheHamDirector->StartStopVisualizer(true, 0);
    TheMaster->GetAudio()->SetPaused(true);
    TheGame->SetTimePaused(true);
    TheUI->GotoScreen("loading_screen", false, false);
}

void Game::SetIntroRealTime(float f) {
    TheTaskMgr.SetSeconds(f, true);
    mHasIntro = f < 0;
    unk60 = true;
    mGameInput->SetTimeOffset();
    TheGamePanel->ResetJitter();
}

void Game::PostLoad() {
    WorldDir *world = TheHamDirector->GetWorld();
    MILO_ASSERT(world, 0x259);
    mMoveDir = world->Find<MoveDir>("moves");
    RELEASE(mOvershell);
    mOvershell = new Overshell();
    mOvershell->Init();
    if (mMoveDir)
        mMoveDir->SetMoveOverlay(sMoveOverlayToggle);
}

void Game::CheckPauseRequest() {
    unk71 = false;
    if (unk72) {
        SetGamePaused(true, unk73, unk74);
        unk72 = false;
    }
}

void Game::LoadNewSongAudio(Symbol s) {
    if (unka0 != s) {
        unka0 = s;
        HamSongDataValidate hsvd = (HamSongDataValidate)0;
        static Symbol dcimindcontrol("dcimindcontrol");
        if (s != dcimindcontrol) {
            MILO_LOG("new_songaudio_name = '%s'\n", s.Str());
            int songID = TheHamSongMgr.GetSongIDFromShortName(s);
            const HamSongMetadata *pMetadata = TheHamSongMgr.Data(songID);
            if (pMetadata->IsOnDisc()) {
                hsvd = (HamSongDataValidate)2;
            }
        }
        RELEASE(mSongInfo);
        mSongInfo = new SongInfoCopy(TheHamSongMgr.SongMgr::SongAudioData(s));
        mMaster->Load(mSongInfo, false, 0, false, hsvd, 0);
        Fader *fader = TheSynth->Find<Fader>("per_song_sfx_level.fade", false);
        if (fader) {
            fader->SetVolume(0);
        }
    }
}

void Game::FlushMoveRecord() {
    MILO_ASSERT(mMoveDir, 0x3a7);
    mMoveDir->FlushMoveRecord();
}

void Game::SwapMoveRecord() {
    MILO_ASSERT(mMoveDir, 0x3af);
    mMoveDir->SwapMoveRecord();
}

void Game::ReloadSong() {
    WorldDir *world = TheHamDirector->GetWorld();
    MILO_ASSERT(world, 0x1c7);
    mMoveDir = world->Find<MoveDir>("moves");
    unk90 = 0;
    LoadSong();
}

bool Game::IsReady() { return IsLoaded() != false; }

void Game::Restart(bool b) {
    unk58++;
    TheGamePanel->ResetJitter();
    TheSynth->StopAllSfx(false);
    TheSynth->StopAllSounds();
    if (b) {
        mMaster->Reset();
    }
    if (unka4 != 5) {
        unka4 = 3;
    }
    if (TheHamDirector)
        TheHamDirector->ResetFacialAnimation();
}

void Game::SetTimePaused(bool b) {
    unk5f = b;
    SetPaused(b, true);
    if (!b && unk60) {
        mGameInput->SetTimeOffset();
    }
}

void Game::PostWaitStart() {
    if (!mMaster->GetAudio()->Fail()) {
        static Symbol gameplay_mode("gameplay_mode");
        static Symbol just_intro("just_intro");
        if (TheHamProvider->Property(gameplay_mode, true)->Sym() == just_intro) {
            mMaster->GetAudio()->SetMuteMaster(true);
        }
        mMaster->GetAudio()->Play();
        unk5e = false;
        MetaPerformer::Current()->StartGameplayTimer();
        unk60 = false;
    }
}

void Game::SetMusicVolume(float vol) {
    MILO_ASSERT(mMaster->GetAudio(), 0x432);
    mMaster->GetAudio()->SetMasterVolume(vol);
}

void Game::StartIntro() {}

void Game::SetHamMove(int i1, HamMove *move, bool b3) {
    if (mMoveDir) {
        HamMove *current = mMoveDir->CurrentMove(i1);
        int i5 = TheTaskMgr.CurrentMeasure();
        if (current) {
            if (TheTaskMgr.CurrentBeat() == 0) {
                i5 = TheTaskMgr.CurrentMeasure() - 1;
            } else if (TheTaskMgr.CurrentBeat() != 3) {
                current->IsRest();
            }
            MILO_ASSERT(TheGameData, 0x2C8);
            HamPlayerData *player_data = TheGameData->Player(i1);
            MILO_ASSERT(player_data, 0x2CA);
            if (player_data->IsPlaying()) {
                float frac = mMoveDir->DetectFrac(i1, i5);
                static Message move_passed("move_passed", -1, 0, 0, 0);
                move_passed[0] = i1;
                move_passed[1] = current;
                move_passed[2] = frac;
                move_passed[3] = b3;
                TheGamePanel->Handle(move_passed, false);
            }
        }
        mMoveDir->SetCurrentMove(i1, move);
    }
}

void Game::SetRealTime(bool b1) {
    unk60 = b1;
    if (unk60) {
        mGameInput->SetTimeOffset();
    }
}

EndGameResult Game::GetResult(bool) {
    EndGameResult res = (EndGameResult)1;
    if (MetaPerformer::Current()->SongEndsWithEndgameSequence()) {
        res = (EndGameResult)2;
    }
    return res;
}

void Game::ResetAudio() {
    unka4 = 0;
    mMaster->ResetAudio();
}

void Game::SetLoop(bool b1) {
    if (mMoveDir) {
        mMoveDir->SetDebugLoop(b1);
    }
}

void Game::SetMusicSpeed(float f1) {
    unk6c = f1;
    mMaster->GetAudio()->GetSongStream()->SetSpeed(f1);
}

void Game::Jump(float f1, bool b2) {
    if (b2) {
        mMaster->Jump(f1);
    }
    TheTaskMgr.ResetTaskTime(f1 / 1000.0f, MsToBeat(f1));
    unk9c = f1;
    unka4 = 2;
}

bool Game::IsWaiting() {
    HamAudio *audio = mMaster->GetAudio();
    if (audio->Fail()) {
        return false;
    } else if (unka4 != 0) {
        return true;
    } else if (audio->IsReady()) {
        return false;
    } else {
        return !audio->IsFinished();
    }
}

void Game::Reset() {
    SongPos pos;
    unk60 = false;
    unk5f = false;
    mSongPos = pos;
    mHasIntro = false;
    unk68 = false;
    TheHamDirector->SetPickingDisabled(false);
    for (int i = 0; i < 2; i++) {
        mMoveDir->SetCurrentMove(i, nullptr);
    }
    TheGamePanel->ResetJitter();
    RELEASE(mGameInput);
    mGameInput = new LiveInput(*mMaster->GetAudio());
    TheTaskMgr.SetAVOffset(mGameInput->GetSongToTaskMgrMs() / 1000.0f);
    TheTaskMgr.SetSeconds(0, true);
    mMaster->SetMaps();
}

void Game::SetForegroundVolume(float volume) {
    mMaster->GetAudio()->SetForegroundVolume(volume);
}

void Game::SetBackgroundVolume(float volume) {
    mMaster->GetAudio()->SetBackgroundVolume(volume);
}

float Game::PollShuttle() {
    mShuttle->Poll();
    float ms = mShuttle->Ms();
    mSongPos = mSongDB->CalcSongPos(TheMaster, ms);
    TheTaskMgr.SetSongPos(mSongPos);
    Jump(ms, false);
    return ms;
}

void Game::PostWaitJump() {
    TheGamePanel->ResetJitter();
    if (unk60) {
        mGameInput->SetPostWaitJumpOffset(unk9c);
    }
    if (TheSongSequence.CurrentIndex() > 0) {
        TheHamDirector->VenueEnter(TheHamDirector->GetVenueWorld());
    }
    if (!mHasIntro) {
        PostWaitStart();
    }
}

bool Game::IsSongDefaultPlayerPlaying() {
    static Symbol song("song");
    Symbol symSong = TheGameData->GetSong();
    Symbol symDefaultCharacter = TheHamSongMgr.GetCharacter(symSong);
    Symbol symPrimaryCharacter = TheGameData->Player(0)->Char();
    bool ret = symPrimaryCharacter == symDefaultCharacter;
    MILO_LOG(
        "Game::IsSongDefaultPlayerPlaying() : symSong = '%s' symDefaultCharacter = '%s' symPrimaryCharacter = '%s' ret = %d\n",
        symSong,
        symDefaultCharacter,
        symPrimaryCharacter,
        ret
    );
    return ret;
}

void Game::LoadSong() {
    if (!TheSongSequence.Done() && TheSongSequence.CurrentIndex() < 0) {
        TheSongSequence.DoNext(true, false);
        return;
    }
    Symbol song = TheGameData->GetSong();
    MetaPerformer::Current()->Handle(Message("on_load_song", 0), true);
    unk5d = false;
    static Symbol cascade("cascade");
    if (TheGameMode->Property("use_movegraph")->Int() != 0
        || TheHamProvider->Property("microgame")->Sym() == cascade
        || TheGameMode->Property("battle_mode")->Sym() == cascade) {
        unk5d = true;
    }
    const HamSongMetadata *data =
        TheHamSongMgr.Data(TheHamSongMgr.GetSongIDFromShortName(song));
    HamSongDataValidate v = (HamSongDataValidate)0;
    if (data->IsOnDisc()) {
        v = (HamSongDataValidate)2;
    }
    Fader *fader = TheSynth->Find<Fader>("per_song_sfx_level.fade", false);
    if (fader) {
        fader->SetVolume(0);
    }
    TheMoveMgr->Clear();
    if (unk5d) {
        TheMoveMgr->SetSong(song);
    }
    RELEASE(mSongInfo);
    mSongInfo = new SongInfoCopy(TheHamSongMgr.SongMgr::SongAudioData(song));
    mMaster->Load(mSongInfo, false, 0, false, v, nullptr);
}

void Game::SetPaused(bool b1, bool b2) {
    if (!b1) {
        TheTaskMgr.SetAVOffset(mGameInput->GetSongToTaskMgrMs() / 1000.0f);
    }
    if (b2) {
        mGameInput->SetPaused(b1);
        unk5e = b1;
        if (unk5e) {
            MetaPerformer::Current()->StopGameplayTimer();
        } else {
            MetaPerformer::Current()->StartGameplayTimer();
        }
    }
    if (b1) {
        static Message msg("world_pause");
        TheGamePanel->Export(msg, true);
    } else {
        static Message msg("world_unpause");
        TheGamePanel->Export(msg, true);
    }
}

void Game::SetGamePaused(bool b1, bool b2, bool b3) {
    if (unk71 && b1) {
        unk73 = b2;
        unk74 = b3;
        unk72 = true;
    } else {
        if (!b1 || b3) {
            TheSynth->PauseAllSfx(b1);
        }
        SetPaused(b1, b2);
        unk71 = true;
        if (b1) {
            TheTaskMgr.SetSecondsAndBeat(
                TheTaskMgr.Seconds(TaskMgr::kRealTime), TheTaskMgr.Beat(), false
            );
        } else if (unk60) {
            mGameInput->SetTimeOffset();
        }
    }
}

void Game::CheckForSkeletonLoss(const Skeleton *const (&skeletons)[6]) {
    int numPlaying = 0;
    for (int i = 0; i < 2; i++) {
        if (TheGameData->Player(i)->IsPlaying()) {
            numPlaying++;
        }
    }
    int threshold = 1;
    if (TheHamProvider->Property("requires_2_players")->Int() != 0
        || TheHamProvider->Property("is_in_party_mode")->Int() != 0) {
        threshold = 2;
    }
    if (numPlaying < threshold) {
        PauseForSkeletonLoss();
    }
}

void Game::LoadNewSongMoves(Symbol s1, bool b2) {
    bool loaded = TheGame->IsLoaded();
    Symbol song = TheGameData->GetSong();
    Symbol s50 = TheMaster->GetAudio()->Name();
    if (b2 || song != s1 || s50 != song) {
        TheGameData->SetSong(s1);
        const char *milo = MakeString("%s.milo", TheHamSongMgr.SongPath(s1, 0));
        if (b2) {
            static Symbol song("song");
            FileMerger *fm = TheHamDirector->GetWorld()->Find<FileMerger>("world.fm");
            FileMerger::Merger *merger = fm->FindMerger(song, true);
            merger->Clear(true);
            merger->SetSelected(milo, true);
            fm->StartLoad(true);
        }
    }
}

void Game::LoadNewSong(Symbol s1, Symbol s2) {
    bool loaded = TheGame->IsLoaded();
    if (s2.Null()) {
        s2 = s1;
    }
    unka4 = 5;
    if (loaded) {
    }
    unk5d = false;
    static Symbol cascade("cascade");
    static Symbol holla_back("holla_back");
    unk5d = TheGameMode->Property("use_movegraph")->Int();
    if (s1 != s2) {
        RELEASE(mSongInfo);
        mSongInfo = new SongInfoCopy(TheHamSongMgr.SongMgr::SongAudioData(s2));
        mMaster->LoadOnlySongData(mSongInfo, true, (HamSongDataValidate)0);
        MultiTempoTempoMap *other =
            static_cast<MultiTempoTempoMap *>(HamSongData::sInstance->GetTempoMap());
        unkac = new MultiTempoTempoMap(*other);
    } else {
        RELEASE(unkac);
    }
    LoadNewSongAudio(s1);
    Symbol s48(TheMaster->GetAudio()->Name());
    LoadNewSongMoves(s2, true);
    if (unk5d) {
        TheMoveMgr->SetSong(s2);
    } else {
        TheMoveMgr->Graph().Clear();
    }
    unk90 = 0;
}

void Game::PauseForSkeletonLoss() {
    if (!unk5e) {
        int gestureVal = TheGestureMgr->GetVal425C();
        if (gestureVal != 0 && gestureVal != 1 && !TheSynth->HasPendingVoices()
            && !TheUI->InTransition()) {
            static Message pauseOnSkeletonLossMsg("pause_on_skeleton_loss");
            DataNode handled = TheGamePanel->HandleType(pauseOnSkeletonLossMsg);
            if (handled.Type() == kDataInt && handled.Int() <= 0) {
                return;
            } else {
                static Message pause_game("pause_game");
                TheGamePanel->Handle(pause_game, true);
            }
        }
    }
}
bool Game::IsLoaded() {
    if (unk90 == 3) {
        return true;
    } else {
        if ((int)mMaster && !mMaster->IsLoaded()) {
            return false;
        }
        if (unk90 == 0) {
            if (!mMaster->IsLoaded()) {
                return false;
            }
            if (unk5d && !TheHamDirector->IsWorldLoaded()) {
                return false;
            }
            TheSongDB->PostLoad(mMaster->GetMidiParserMgr()->GetEventsList());
            PostLoad();
            if (unk5d) {
                MILO_ASSERT(mMoveDir, 0x224);
                ObjectDir *moveData = mMoveDir->Find<ObjectDir>("move_data", false);
                MILO_ASSERT_FMT(
                    moveData,
                    "move_data.milo is not in moves.milo; re-run update_move_data_proxy.dta"
                );
                TheMoveMgr->LoadMoveData(moveData);
                SuperEasyRemixer::LoadAllVariants();
            } else {
                MILO_LOG("Game::IsLoaded() - not using MoveGraph");
            }
            unk90 = 1;
        }
        if (unk90 == 1) {
            if (unk5d && !TheHamDirector->IsMoveMergerFinished()) {
                return false;
            }
            MILO_LOG("Game::IsLoaded() - Done waiting for MoveGraph\n");
            unk90 = 2;
        }
        if (unk90 == 2) {
            if (mMaster->GetAudio()->Fail()) {
                return true;
            }
            if (!mMaster->GetAudio()->IsReady()) {
                TheSynth->Poll();
                return false;
            }
            unk90 = 3;
            TheProfileMgr.PushAllOptions();
        }
        return unk90 == 3;
    }
}

DataNode Game::OnSetShuttle(DataArray *a) {
    if (a->Size() > 3) {
        mShuttle->SetController(a->Int(3));
    }
    bool active = a->Int(2);
    if (active) {
        float time = mMaster->GetAudio()->GetTime();
        mShuttle->SetMs(time);
        mShuttle->SetEndMs(mSongDB->GetSongDurationMs());
    } else {
        Jump(mShuttle->Ms(), true);
        while (!IsLoaded()) {
            TheSynth->Poll();
        }
    }
    mShuttle->SetActive(active);
    return 0;
}

DataNode Game::OnResetDetection(DataArray *a) {
    MILO_ASSERT(mMoveDir, 0x392);
    if (a->Size() > 2) {
        int index = a->Int(2);
        HamPlayerData *player_data = TheGameData->Player(index);
        MILO_ASSERT(player_data, 0x398);
        MILO_ASSERT(player_data->IsPlaying(), 0x399);
        mMoveDir->ResetDetectFrames(index, player_data->GetDifficulty());
    } else {
        mMoveDir->ResetDetection();
    }
    return 0;
}

DataNode OnToggleMoveOverlay(DataArray *a) {
    sMoveOverlayToggle = !sMoveOverlayToggle;
    if (TheGame && TheGame->GetMoveDir()) {
        TheGame->GetMoveDir()->SetMoveOverlay(sMoveOverlayToggle);
    }
    return sMoveOverlayToggle ? "ON" : "OFF";
}

DataNode OnToggleAutoplay(DataArray *a) {
    HamPlayerData *player_data = TheGameData->Player(a->Int(1));
    MILO_ASSERT(player_data, 0x6F);
    if (!player_data->IsAutoplaying()) {
        player_data->SetAutoplay(sAutoplayStates[0]);
    } else {
        player_data->SetAutoplay(gNullStr);
    }
    return player_data->IsAutoplaying();
}

DataNode OnCycleAutoplay(DataArray *);

DataNode OnToggleCharFeedback(DataArray *a) {
    ReserveFrames();
    return CharFeedback::sEnabled = !CharFeedback::sEnabled;
}

DataNode OnToggleSongRecord(DataArray *a) {
    ReserveFrames();
    MoveDir::sGameRecord2Player = false;
    return MoveDir::sGameRecord = !MoveDir::sGameRecord;
}

DataNode OnToggleSongRecordDouble(DataArray *a) {
    MoveDir::sGameRecord2Player = !MoveDir::sGameRecord2Player;
    MoveDir::sGameRecord = MoveDir::sGameRecord2Player;
    return MoveDir::sGameRecord;
}

DataNode OnCycleTestDancer(DataArray *);
DataNode OnDumpMoves(DataArray *);

void GameInit() {
    GameModeInit();
    REGISTER_OBJ_FACTORY(GamePanel)
    REGISTER_OBJ_FACTORY(BustAMovePanel)
    TheDebug.AddExitCallback(GameTerminate);
    TheSongSequence.Init();
    sAutoplayStates.push_back("maximum");
    for (int i = 0; i < 4; i++) {
        sAutoplayStates.push_back(RatingState(i));
    }
    DataRegisterFunc("toggle_move_overlay", OnToggleMoveOverlay);
    DataRegisterFunc("toggle_autoplay", OnToggleAutoplay);
    DataRegisterFunc("cycle_autoplay", OnCycleAutoplay);
    DataRegisterFunc("toggle_char_feedback", OnToggleCharFeedback);
    DataRegisterFunc("toggle_song_record", OnToggleSongRecord);
    DataRegisterFunc("toggle_song_record_double", OnToggleSongRecordDouble);
    DataRegisterFunc("cycle_test_dancer", OnCycleTestDancer);
    DataRegisterFunc("dump_moves", OnDumpMoves);
}

void GameTerminate() {
    TheHamSongMgr.Terminate();
    GameModeTerminate();
}
