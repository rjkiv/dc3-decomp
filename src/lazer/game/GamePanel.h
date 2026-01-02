#pragma once
#include "game/Game.h"
#include "gesture/FitnessFilter.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Timer.h"
#include "rndobj/Overlay.h"
#include "ui/UIPanel.h"
#include "utl/Profiler.h"

class GamePanel : public UIPanel {
public:
    enum State {
        kGameInIntro = 1,
        kGamePlaying = 2,
        kGameOver = 3,
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

    Game *mGame; // 0x38
    FitnessFilter mFitnessFilters[2]; // 0x3c
    RndOverlay *mTimeOverlay; // 0x6c
    RndOverlay *mLatencyOverlay; // 0x70
    RndOverlay *mFitnessOverlay; // 0x74
    RndOverlay *mLoopVizOverlay; // 0x78
    bool unk7c;
    State mState; // 0x80
    int unk84;
    Profiler unk88;
    bool unkd8;
    std::vector<float> unkdc;
    int unke8;
    int unkec;
    float unkf0;
    int unkf4;
    bool unkf8;
    Timer *unkfc;
    bool unk100;
    bool unk101;
    int unk104;
    bool unk108;
};

extern GamePanel *TheGamePanel;

class LatencyCallback : public RndOverlay::Callback {
public:
    virtual ~LatencyCallback() {}
    virtual float UpdateOverlay(RndOverlay *o, float y);
};

class LoopVizCallback : public RndOverlay::Callback {
public:
    LoopVizCallback();
    virtual ~LoopVizCallback() {}
    virtual float UpdateOverlay(RndOverlay *o, float y);

private:
};
