#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
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
    void PollCaptureFlashcard();
    void AnimateFlashcard(int);
    void AdvanceFlashcards();
    int RepsToNextPhrase();
    void SetFlashcardImage(int, int, int);

    BAMState mState; // 0x3c
    FreestyleMoveRecorder *mRecorder; // 0x40
    int mReps; // 0x44
    std::list<Symbol> mFlashcardText; // 0x48
    std::list<int> mFlashcardImage; // 0x50
    int mRecordedSkeletonIndex; // 0x58
    float mMoveScore; // 0x5c
    ObjectDir *mHudPanel; // 0x60
    int mRecordingPlayer; // 0x64
    int mStartOffset; // 0x68
    int mSuccesses; // 0x6c
    BAMState mQueueState; // 0x70 - BAMState
    HamLabel *mStatus; // 0x74
    HamLabel *mMovePrompt; // 0x78
    bool mBustedMoveSuccessfully; // 0x7c
    float mRecordTime; // 0x80
    int mNumCreatedMoves; // 0x84
    int mSequenceSkelIndex[2]; // 0x88
    float mSequenceMoveScore[2]; // 0x90
    RndDir *mPlayerColumn[kNumSkeletonSides]; // 0x98
    SkeletonSide mActiveSide; // 0xa0
    DancerSkeleton mFlashcardPose[3]; // 0xa4
    int mCaptureFlashcard; // 0x92c
    float mCaptureFlashcardTimer; // 0x930
    int mRenderFlashcard; // 0x934
    HamPanel *mVisualizer; // 0x938
    int mFlashcardName[4]; // 0x93c
    int mBustFailures[2]; // 0x94c
    int mAllowedBustFailures; // 0x954
    float mSongLoopStart; // 0x958
    float mSongLoopEnd; // 0x95c
    HamPhraseMeter *mPhraseMeter[kNumSkeletonSides]; // 0x960
    int mHideTransitionOnBeat; // 0x968
    int mFinalSequenceType; // 0x96c
    bool mUsingMulligan; // 0x970
    float mDuringBustMoveRatings[2]; // 0x974
    std::vector<int> mSongStructure; // 0x97c
    int mRepsLeft; // 0x988
    bool mStreamJumped; // 0x98c
    std::vector<int> mShuffledMoveNames; // 0x990
    int mShuffledMoveNameIndex; // 0x99c
    float mPlayMovePromptAt; // 0x9a0
    bool mHasFlawlessedAllMoves[2]; // 0x9a4
    int mMoveCreator[4]; // 0x9a8
    bool mNeedToPlayIntroVO; // 0x9b8
    bool unk9b9;
    int unk9bc;
};
