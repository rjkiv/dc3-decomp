#include "game/Game.h"
#include "Game.h"
#include "SongDB.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "game/Shuttle.h"
#include "game/SongDB.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamSongData.h"
#include "hamobj/MoveDir.h"
#include "macros.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/Overshell.h"
#include "midi/MidiParserMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "synth/Faders.h"
#include "synth/Synth.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/SongInfoCopy.h"
#include "utl/Symbol.h"
#include "world/Dir.h"

Game *TheGame;

Game::Game()
    : unk30(0), unk34(0), unk38(0), unk3c(0), unk40(0), unk44(0), unk48(new SongDB()),
      unk58(0), unk5c(false), unk5d(false), unk5e(true), unk5f(false), unk60(false),
      unk64(0), unk68(false), unk70(false), unk71(false), unk78(0), mMoveDir(this),
      unk90(0), unka4(), unka8(), unkac() {
    if (TheSongDB) {
        delete TheSongDB;
    }
    TheSongDB = unk48;
    TheGame = this;
    SetName("game", ObjectDir::Main());
    TheMaster = new HamMaster(unk48->unk0, TheMidiParserMgr);
    mMaster = TheMaster;
    TheMaster->SetName("game", ObjectDir::Main());
    LoadSong();
    unk72 = false;
    unk73 = false;
    unk74 = true;
    SkeletonUpdate::InstanceHandle();
}

Game::~Game() {}

BEGIN_PROPSYNCS(Game)
    SYNC_PROP_SET(music_speed, unk6c, SetMusicSpeed(_val.Float()))
END_PROPSYNCS

void Game::Start() {
    mHasIntro = false;

    if (unka4 == 3) {
        unka4 = 4;
    } else
        unka4 = 1;
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
    TheTaskMgr.SetSeconds(f, false);
    mHasIntro = f < 0;
    unk60 = true;
    // GamePanel
}

void Game::PostLoad() {
    WorldDir *world = TheHamDirector->GetWorld();
    MILO_ASSERT(world, 0x259);
    mMoveDir = world->Find<MoveDir>("moves");
    RELEASE(unk78);
    unk78 = new Overshell();
    unk78->Init();
    if (mMoveDir)
        mMoveDir->SetMoveOverlay(false); // re-check this parameter
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
            MILO_LOG("new_songaudio_name = \'%s\'\n", s);
            int songID = TheHamSongMgr.GetSongIDFromShortName(s);
            const HamSongMetadata *pMetadata = TheHamSongMgr.Data(songID);
            if (pMetadata->IsOnDisc()) {
                hsvd = (HamSongDataValidate)2;
            }
        }
        RELEASE(unk4c);
        mMaster->Load(0, false, 0, false, hsvd, 0);
        Fader *f = TheSynth->Find<Fader>("per_song_sfx_level.fade", false);
        if (f)
            f->SetVolume(0);
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

bool Game::IsReady() { return !IsLoaded() == false; }

void Game::Restart(bool b) {
    unk58++;
    // GamePanel call
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
        // idk what 0x54 is
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

void GameTerminate() {
    TheHamSongMgr.Terminate();
    GameModeTerminate();
}
