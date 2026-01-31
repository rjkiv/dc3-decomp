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

class Game : public Hmx::Object, public SkeletonCallback {
public:
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
    int unk58;
    bool unk5c;
    bool unk5d; // 0x5d - cascade?
    bool unk5e;
    bool unk5f;
    bool unk60;
    bool unk61;
    bool mHasIntro; // 0x62
    float unk64;
    bool unk68;
    bool unk69;
    float unk6c;
    bool unk70;
    bool unk71;
    bool unk72;
    bool unk73;
    bool unk74;
    Overshell *mOvershell; // 0x78
    ObjPtr<MoveDir> mMoveDir; // 0x7c
    int unk90;
    Shuttle *mShuttle; // 0x94
    int unk98;
    float unk9c;
    Symbol unka0;
    int unka4; // 0xa4 - state?
    int unka8;
    MultiTempoTempoMap *unkac;
};

void GameInit();
void GameTerminate();

extern Game *TheGame;
