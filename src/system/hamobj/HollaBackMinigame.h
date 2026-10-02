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

class HollaBackMinigame : public RndPollable {
public:
    enum State {
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

    int unk8; // 0x8
    int unkc; // 0xc
    Symbol mMoveStates[256]; // 0x10
    bool unk410; // 0x410
    int mState; // 0x414 - state
    int unk418;
    int mNumMoves; // 0x41c
    int unk420;
    int mSpecifyFirstMoveMeasure; // 0x424
    Symbol mIntroShoutout; // 0x428
    std::vector<Symbol> mWinShoutouts; // 0x42c
    std::vector<Symbol> mWinCamCuts; // 0x438
    bool unk444;
    int mInitialMoveCount; // 0x448
    int mMaxRoutineSize; // 0x44c
    float unk450;
    PanelDir *mHUDPanel; // 0x454
    RndDir *mHollabackHUD; // 0x458
    UIPanel *mFlashcardDockPanel; // 0x45c
    ObjectDir *unk460; // 0x460
    RndDir *mScoreLeft; // 0x464
    RndDir *mScoreRight; // 0x468
    bool unk46c;
    float mInitialMusicJump; // 0x470
    bool unk474;
    bool unk475;
    Sound *mSound; // 0x478
    int unk47c;
    bool unk480;
    bool unk481;
    int unk484;
    std::vector<HamMove *> unk488;
    int unk494;
};
