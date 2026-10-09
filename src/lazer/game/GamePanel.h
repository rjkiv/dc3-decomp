#pragma once
#include "game/Game.h"
#include "gesture/FitnessFilter.h"
#include "meta_ham/MetaPerformer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Timer.h"
#include "rndobj/Overlay.h"
#include "ui/UIPanel.h"
#include "utl/DebugMeter.h"
#include "utl/Profiler.h"

class GamePanel : public UIPanel {
public:
    enum GameState {
        kGameNeedIntro = 0x0000,
        kGameNeedStart = 0x0001,
        kGamePlaying = 0x0002,
        kGameOver = 0x0003,
    };

    enum LoadingState {
        kLoadingState_NotReady = 0x0000,
        kLoadingState_UILoaded = 0x0001,
        kLoadingState_WorldLoaded = 0x0002,
        kLoadingState_CharsLoaded = 0x0003,
        kLoadingState_Ready = 0x0004,
    };

    GamePanel();
    // Hmx::Object
    virtual ~GamePanel();
    OBJ_CLASSNAME(GamePanel);
    OBJ_SET_TYPE(GamePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void SetTypeDef(DataArray *);
    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();
    virtual void SetPaused(bool);
    virtual void FinishLoad();

    NEW_OBJ(GamePanel)

    void SetGameOver(bool);
    bool IsPastStreamJumpPointOfNoReturn();
    void ResetLimbFeedback();
    void SetLimbFeedbackVisible(bool);
    FitnessFilter *GetFitnessFilter(int);
    void ResetJitter();
    float DeJitter(float);

    DataNode OnGetFitnessData(const DataArray *);
    bool IsGameOver() const { return mState == kGameOver; }
    bool GetDriveTime() const { return mDriveTime; }

private:
    void CreateGame();
    void StartGame();
    void SetPausedHelper(bool, bool);
    void CheatPause(bool);
    void Reset();
    void UpdateFitnessOverlay();
    void StartIntro();
    void SetSoundEventReceiver();
    void UpdateNowBar();
    void UpdateLatency();

    DataNode OnStartLoadSong(DataArray *);
    DataNode OnStartSongNow(DataArray *);

protected:
    virtual void Load();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void PollForLoading();

    void ClearDrawGlitch();
    void ReloadData();

    DataNode OnMsg(const EndGameMsg &);

private:
    Game *mGame; // 0x38
    FitnessFilter mFitnessFilter[2]; // 0x3c
    RndOverlay *mNowBar; // 0x6c
    RndOverlay *mLatency; // 0x70
    RndOverlay *mFitness; // 0x74
    RndOverlay *mLoopViz; // 0x78
    bool mStartPaused; // 0x7c
    GamePanel::GameState mState; // 0x80
    EndGameResult mResult; // 0x84
    Profiler mLoadProf; // 0x88
    bool mReplay; // 0xd8
    std::vector<float> mJitter; // 0xdc
    int mJitterIndex; // 0xe8
    int mJitterWindow; // 0xec
    float mLastAverage; // 0xf0
    int mLastMs; // 0xf4
    bool mDriveTime; // 0xf8
    Timer *mPauseCountInTimer; // 0xfc
    bool mShouldCountIn; // 0x100
    bool mCheatPaused; // 0x101
    int mLoadingState; // 0x104
    bool mbSoundEventReceiverSet; // 0x108
};

extern GamePanel *TheGamePanel;

class LatencyCallback : public RndOverlay::Callback {
public:
    LatencyCallback() : unk4(0) {}
    virtual ~LatencyCallback() {}
    virtual float UpdateOverlay(RndOverlay *o, float y);

private:
    bool unk4;
};

class LoopVizCallback : public RndOverlay::Callback {
public:
    LoopVizCallback();
    virtual ~LoopVizCallback() {}
    virtual float UpdateOverlay(RndOverlay *o, float y);

    void DrawHashMarks(float, float, float, int, int, bool);

private:
    DebugMeter mDebugMeter1; // 0x4
    DebugMeter mDebugMeter2; // 0x24
    int unk44;
    int unk48;
    int unk4c;
    int unk50;
    float unk54;
    float unk58;
};
