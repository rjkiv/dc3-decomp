#pragma once
#include "FreestyleMoveRecorder.h"
#include "char/CharClip.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/Ham.h"
#include "hamobj/HamLabel.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"

class PoseFatalities : public Hmx::Object {
public:
    // Hmx::Object
    PoseFatalities();
    virtual ~PoseFatalities() {}
    OBJ_CLASSNAME(PoseFatalities);
    OBJ_SET_TYPE(PoseFatalities);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    NEW_OBJ(PoseFatalities)

    void DrawDebug();
    void Enter();
    bool InFatality(int) const;
    void ActivateFatal(int);
    bool FatalActive() const;
    void SetJump(int, int);
    bool GotFullCombo(int) const;
    void Poll();

private:
    Symbol GetFatalityFace();
    bool InStrikeAPose();
    void SetCombo(int, int);
    void SetFatalitiesStart(int);
    void Reset();
    void PlayVO(Symbol);
    String GetCelebrationClip(int);
    void EndFatal(int);
    float BeatsLeftToMatch(int);
    void BeginFatal(int);
    void AddFatal(int);
    bool CheckMatchingPose(int);
    void LoadFatalityClips();
    void PollVO();
    void OnBeat(int);
    void UpdateClipDriver(int);
    void UpdateMatchingPose(int);
    void OnFatalResult(int, bool);

    bool mFatalActive[2]; // 0x2c
    int mFatalStartBeats[2]; // 0x30 - per player
    int mFatalEndBeat; // 0x38
    int mPoseStartBeat[2]; // 0x3c
    int mFatalityIndex[2]; // 0x44
    Skeleton mPoseSkel[2]; // 0x4c
    float mHoldScore[2]; // 0x15f4
    float mHoldDuration; // 0x15fc - hold duration?
    FreestyleMoveRecorder mRecorder; // 0x1600
    float mSatisfaction[2]; // 0x1710
    float mClipOffset[2]; // 0x1718
    bool mFatalOver[2]; // 0x1720
    std::list<CharClip *> mAllFatalityClips; // 0x1724
    int mFatalityBeatLeadIn; // 0x172c
    HamLabel *mComboLabel[kNumSkeletonSides]; // 0x1730
    int mComboLevel[2]; // 0x1738
    bool mGotFullCombo[2]; // 0x1740
    ObjectDir *mHudPanel; // 0x1744
    bool mInFatalityMode; // 0x1748
    int mJumpStart; // 0x174c
    int mJumpEnd; // 0x1750
    int mBeat; // 0x1754 - some sort of beat
    RndAnimatable *mPoseBeatAnims[kNumSkeletonSides]; // 0x1758
    float mEndPoseFatalitiesIn; // 0x1760
    float mVOCooldownTime; // 0x1764
    unsigned int mVORequests; // 0x1768 - flags/mask
    float mTimeSinceVORequest;
};
