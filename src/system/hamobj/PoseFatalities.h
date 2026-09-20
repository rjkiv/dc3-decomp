#pragma once
#include "FreestyleMoveRecorder.h"
#include "char/CharClip.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/HamLabel.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"

#define MAX_NUM_PLAYERS 2
#define NUM_FATALITIES 8

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

    static SkeletonViz *sViz1;
    static SkeletonViz *sViz2;

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

    bool mInFatality[2]; // 0x2c
    int mFatalStartBeats[2]; // 0x30 - per player
    int mFatalEndBeat; // 0x38
    int unk3c[2];
    int unk44[2];
    Skeleton mPlayerSkeletons[2]; // 0x4c
    float unk15f4[2]; // 0x15f4
    float unk15fc; // 0x15fc - hold duration?
    FreestyleMoveRecorder mRecorder; // 0x1600
    float unk1710[2];
    float unk1718[2];
    bool unk1720[2];
    std::list<CharClip *> mAllFatalityClips; // 0x1724
    int mFatalityBeatLeadIn; // 0x172c
    HamLabel *mPoseComboLabels[kNumSkeletonSides]; // 0x1730
    int mCurrentCombo[2]; // 0x1738
    bool mGotFullCombo[2]; // 0x1740
    ObjectDir *mHudPanel; // 0x1744
    bool unk1748;
    int mJumpStart; // 0x174c
    int mJumpEnd; // 0x1750
    int unk1754; // 0x1754 - some sort of beat
    RndAnimatable *mPoseBeatAnims[kNumSkeletonSides]; // 0x1758
    float unk1760;
    float unk1764;
    unsigned int unk1768; // 0x1768 - flags/mask
    float unk176c;
};
