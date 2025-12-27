#pragma once
#include "game/Shuttle.h"
#include "game/SongDB.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamMove.h"
#include "hamobj/MoveDir.h"
#include "meta_ham/Overshell.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/SongInfoCopy.h"
#include "utl/Symbol.h"

enum EndGameResult {
};

class Game : public Hmx::Object, public SkeletonCallback {
public:
    // Hmx::Object
    virtual ~Game();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // SkeletonCallback
    virtual void PostUpdate(SkeletonUpdateData const *);

    Game();
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

protected:
    float unk30;
    float unk34;
    int unk38;
    int unk3c;
    int unk40;
    int unk44;
    SongDB *unk48;
    SongInfo *unk4c;
    HamMaster *mMaster; // 0x50
    u32 unk54;
    int unk58;
    bool unk5c;
    bool unk5d;
    bool unk5e;
    bool unk5f;
    bool unk60;
    bool unk61;
    bool mHasIntro; // 0x62
    float unk64;
    bool unk68;
    u8 unk69;
    float unk6c;
    bool unk70;
    bool unk71;
    bool unk72;
    bool unk73;
    bool unk74;
    Overshell *unk78;
    ObjPtr<MoveDir> mMoveDir; // 0x7c
    int unk90;
    Shuttle unk94;
    Symbol unka0;
    int unka4;
    int unka8;
    int unkac;

private:
    void PostWaitStart();
    float PollShuttle();
    void PostWaitJump();
    void PostWaitRestart();
    DataNode OnResetDetection(DataArray *);
    void PostLoad();
    void SetHamMove(int, HamMove *, bool);
    void PauseForSkeletonLoss();
    void SetPaused(bool, bool);
    bool HandleWait();
    void CheckForSkeletonLoss(class Skeleton const *const (&)[6]);
    DataNode OnSetShuttle(DataArray *);
};

void GameTerminate();

extern Game *TheGame;
