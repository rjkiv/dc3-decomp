#pragma once
#include "gesture/BaseSkeleton.h"
#include "hamobj/BustAMoveData.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/ScoreUtl.h"
#include "lazer/meta_ham/HamPanel.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"

class BustAMovePanel : public HamPanel {
public:
    enum BAMState {
        kBAMState_CountIn = 0,
        kBAMState_Recording = 1,
        kBAMState_Playing = 2,
        kBAMState_ShowMove = 3,
        kBAMState_PlayCountIn = 4,
        kBAMState_RecordCountIn = 5,
        kBAMState_FailureToBust = 6,
        kBAMState_ShowMoveSequenceSetup = 7,
        kBAMState_ShowMoveSequence = 8,
        kBAMState_End = 9,
        kBAMState_None = 10,
    };
    BustAMovePanel();
    // Hmx::Object
    virtual ~BustAMovePanel();
    OBJ_CLASSNAME(BustAMovePanel);
    OBJ_SET_TYPE(BustAMovePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // UIPanel
    virtual void Draw();
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();

    NEW_OBJ(BustAMovePanel)

    void OnBeat();
    void SetUpSongStructure(Symbol);
    void PlayVO(Symbol);

private:
    bool InBustAMove();

    void CacheObjects();
    float GetMovePromptVOLength();
    void PlayIntroVO();
    void QueueMovePromptVO();
    void PlayMovePromptVO();
    DataArray *GetMoveNameData(int);
    Symbol GetPlayerColor(int);
    MoveRating GetMoveRating(float);
    void SetFlashcardText(int, int, Symbol);
    void SetMovePrompt();
    void IncreaseScore(int, int);
    void ResetScores();
    void SetFlashcardName(int, int, int);
    void CountIn(int);
    void ShowMoveRating(MoveRating, int);
    void SetRoundFailure();
    void ShowGetReadyCard(Symbol, SkeletonSide);
    void SetUpMoveNames();

    BAMState mState; // 0x3c
    FreestyleMoveRecorder *unk40;
    int unk44;
    std::list<Symbol> unk48;
    std::list<int> unk50;
    int unk58; // FreestyleMoveRecorder*
    float unk5c;
    ObjectDir *mHUDPanel; // 0x60
    int unk64;
    int unk68;
    int unk6c;
    int unk70;
    HamLabel *mStatusLabel; // 0x74
    HamLabel *mMovePromptLabel; // 0x78
    bool unk7c;
    float unk80;
    int unk84;
    int unk88;
    int unk8c;
    int unk90;
    int unk94;
    RndDir *mBAMColumns[kNumSkeletonSides]; // 0x98
    int unka0;
    DancerSkeleton unka4[3]; // 0xa4
    int unk92c;
    float unk930;
    int unk934;
    HamPanel *mBAMVisualizerPanel; // 0x938
    int unk93c[4];
    int unk94c;
    int unk950;
    int unk954;
    float unk958;
    float unk95c;
    HamPhraseMeter *mPhraseMeters[kNumSkeletonSides]; // 0x960
    int unk968;
    int unk96c;
    bool unk970;
    int unk974;
    int unk978;
    std::vector<int> mSongStructure; // 0x97c
    int unk988; // 0x988 - num reps
    bool unk98c;
    std::vector<int> mShuffledMoveNames; // 0x990
    int unk99c;
    float unk9a0;
    int unk9a4;
    int unk9a8;
    int unk9ac;
    int unk9b0;
    int unk9b4;
    bool unk9b8;
    bool unk9b9;
    int unk9bc;
};
