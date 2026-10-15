#pragma once
#include "hamobj/HamMove.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Poll.h"
#include "synth/Sound.h"
#include "ui/PanelDir.h"
#include "ui/UIPanel.h"
#include "utl/MemMgr.h"

/** "Decode the Craze" minigame in campaign mode. */
class HollaBackMinigame : public RndPollable {
public:
    enum State {
        Title = 0,
        Instruction = 1,
        Win = 2,
        Unk = -1 // See HollaBackMinigame::BeginMinigame (L307)
    };
    // Hmx::Object
    virtual ~HollaBackMinigame();
    OBJ_CLASSNAME(HollaBackMinigame);
    OBJ_SET_TYPE(HollaBackMinigame);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter() { RndPollable::Enter(); }
    virtual void Exit() { RndPollable::Exit(); }

    void BeginMinigame(DataArray *);
    void EndMinigame(bool);
    void OnBeat();
    Symbol GetMoveState(int) const;
    void SetMoveState(int, Symbol);
    void SetDefaultShot();
    void EndShoutOut();
    void SetNumMoves(int);
    void SetState(State);
    void StartShoutOut(const char *);
    void DecipherShoutOut(float);
    void WinShoutOut();
    float NailedMovesInRoutinePct();

    OBJ_MEM_OVERLOAD(0x21)
    NEW_OBJ(HollaBackMinigame)

protected:
    HollaBackMinigame();

    int mStartLoopMeasure; // 0x8
    int mEndLoopMeasure; // 0xc
    Symbol mMoveStates[256]; // 0x10
    bool mActive; // 0x410
    State mState; // 0x414 - state
    int mStateBeats; // 0x418
    int mNumMoves; // 0x41c
    int mMeasure; // 0x420
    int mSpecifyFirstMoveMeasure; // 0x424
    Symbol mIntroShoutout; // 0x428
    std::vector<Symbol> mWinShoutouts; // 0x42c
    std::vector<Symbol> mWinCamCuts; // 0x438
    bool mHaveDoneInitialShoutout; // 0x444
    int mInitialMoveCount; // 0x448
    int mMaxRoutineSize; // 0x44c
    float mTotalMeasures; // 0x450
    PanelDir *mHUDPanel; // 0x454
    RndDir *mHollabackHUD; // 0x458
    UIPanel *mFlashcardDockPanel; // 0x45c
    ObjectDir *mFlashcardDock; // 0x460
    RndDir *mDirScoreLeft; // 0x464
    RndDir *mDirScoreRight; // 0x468
    bool mHaveStartedInstruction; // 0x46c
    float mInitialMusicJump; // 0x470
    bool mInitialJumpPending; // 0x474
    bool mNailedLast; // 0x475
    Sound *m_ShoutoutSound; // 0x478
    int m_WinLingerBeats; // 0x47c
    bool m_FirstInstruction; // 0x480
    bool m_HaveBegunIntroVO; // 0x481
    int m_iRoundOn; // 0x484
    std::vector<HamMove *> mUniqueMoves; // 0x488
    int m_PlayIntroVODelay; // 0x494
};
