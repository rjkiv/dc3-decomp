#pragma once
#include "game/GameInput.h"
#include "game/Shuttle.h"
#include "game/SongDB.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamMove.h"
#include "hamobj/MoveDir.h"
#include "meta_ham/Overshell.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Msg.h"
#include "utl/MultiTempoTempoMap.h"
#include "utl/SongInfoCopy.h"
#include "utl/SongPos.h"
#include "utl/Symbol.h"
#include "meta_ham/MetaPerformer.h"

struct GamePauseRequest { /* Size=0x3 */
public:
    bool mRequest; // 0x0
    bool mPlaying; // 0x1
    bool mPauseSfx; // 0x2
};

class Game : public Hmx::Object, public SkeletonCallback {
public:
    enum LoadState {
        kLoadingSong = 0x0000,
        kWaitingForMoveGraph = 0x0001,
        kWaitingForAudio = 0x0002,
        kReady = 0x0003
    };

    enum WaitState {
        kWaitNone = 0x0000,
        kWaitStart = 0x0001,
        kWaitJump = 0x0002,
        kWaitRestart = 0x0003,
        kWaitRestartAndStart = 0x0004,
        kWaitNewSong = 0x0005,
    };

    Game();
    // Hmx::Object
    virtual ~Game();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // SkeletonCallback
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    void Start();
    bool HasIntro();
    void SetMusicSpeed(float);
    void SetIntroRealTime(float);
    EndGameResult GetResult(bool);
    void Jump(float, bool);
    void Restart(bool);
    bool IsWaiting();
    void Reset();
    void FlushMoveRecord();
    void SwapMoveRecord();
    void SetMusicVolume(float);
    void CheckPauseRequest();
    void ClearState();
    bool IsSongDefaultPlayerPlaying();
    void LoadNewSongAudio(Symbol);
    void LoadSong();
    void ReloadSong();
    bool IsLoaded();
    bool IsReady();
    void Poll();
    void SetTimePaused(bool);
    void LoadNewVenue(Symbol);
    void LoadNewSongMoves(Symbol, bool);
    void SetGamePaused(bool, bool, bool);
    void LoadNewSong(Symbol, Symbol);
    void StartIntro();
    void SetRealTime(bool);
    void ResetAudio();
    void SetLoop(bool);
    void SetForegroundVolume(float);
    void SetBackgroundVolume(float);
    int GetNumRestarts() const;

    MoveDir *GetMoveDir() const { return mMoveDir; }
    HamMaster *GetMaster() const { return mMaster; }
    bool Paused() const { return mPaused; }
    bool PauseTime() const { return mPauseTime; }

private:
    void PostWaitStart();
    float PollShuttle();
    void PostWaitJump();
    void PostWaitRestart();
    void PostLoad();
    void SetHamMove(int, HamMove *, bool);
    void PauseForSkeletonLoss();
    void SetPaused(bool, bool);
    bool HandleWait();
    void CheckForSkeletonLoss(class Skeleton const *const (&)[6]);

    DataNode OnSetShuttle(DataArray *);
    DataNode OnResetDetection(DataArray *);

    SongPos mSongPos; // 0x30
    SongDB *mSongDB; // 0x48
    SongInfo *mSongInfo; // 0x4c
    HamMaster *mMaster; // 0x50
    GameInput *mGameInput; // 0x54
    int mNumRestarts; // 0x58
    bool mCurrentMoveDetected; // 0x5c
    bool mUsingMoveGraph; // 0x5d
    bool mPaused; // 0x5e
    bool mPauseTime; // 0x5f
    bool mRealtime; // 0x60
    bool mRestartedYet; // 0x61
    bool mHasIntro; // 0x62
    float mLastPollMs; // 0x64
    bool mBroadcastIntroEnd; // 0x68
    float mMusicSpeed; // 0x6c
    bool mNeverAllowInput; // 0x70
    bool mSetPausedCalled; // 0x71
    GamePauseRequest mGamePauseRequest; // 0x72
    Overshell *mOvershell; // 0x78
    ObjPtr<MoveDir> mMoveDir; // 0x7c
    Game::LoadState mLoadState; // 0x90
    Shuttle *mShuttle; // 0x94
    bool mUseOldWait; // 0x98
    float mJumpWaitMs; // 0x9c
    Symbol mOldSongAudioName; // 0xa0
    Game::WaitState mWaitState; // 0xa4
    Game::WaitState mPrevWaitState; // 0xa8
    MultiTempoTempoMap *mMovesTempoMap; // 0xac
};

void GameInit();
void GameTerminate();

extern Game *TheGame;

static inline bool AllPaused() {
    if (!TheGame || TheGame->PauseTime() || TheGame->Paused())
        return false;
    return true;
}
