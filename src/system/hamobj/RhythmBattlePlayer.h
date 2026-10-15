#pragma once
#include "flow/Flow.h"
#include "gesture/DepthBuffer3D.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/TransConstraint.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "rndobj/Part.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

class RhythmBattle;

enum RhythmBattleJackState {
    kNoJack = -1,
    kJackBonusLeft = 3,
    kJackBonusRight = 4,
    kJackPenaltyLeft = 5,
    kJackPenaltyRight = 6,
};

enum RhythmBattleScoreState {
    kNoGroove = 0,
    kFreshMoves = 1,
    kSwitchUp = 2,
};

/** "The state of a player in Rhythm Battle." */
class RhythmBattlePlayer : public RndPollable {
public:
    // Hmx::Object
    virtual ~RhythmBattlePlayer();
    OBJ_CLASSNAME(RhythmBattlePlayer)
    OBJ_SET_TYPE(RhythmBattlePlayer)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(RhythmBattlePlayer)

    int InTheZone() const;
    void SetInTheZone(int, bool, bool);
    float InAnimBeatLength() const;
    void SetWindow(float, float);
    bool UpdateState();
    void HackPlayerQuit();
    int SwagJacked(Hmx::Object *, RhythmBattleJackState);
    void SwagJackedBonus(Hmx::Object *, RhythmBattleJackState, int);
    void AnimateIn();
    void SwapObjs(RhythmBattlePlayer *);
    void OnReset(class RhythmBattle *);
    void UpdateScore(Hmx::Object *);
    void UpdateComboProgress();
    void UpdateAnimations(Hmx::Object *);
    void ResetCombo();
    void SetActive(bool);
    void AnimateOut();
    bool Unk2a8Check() const { return unk2a5 && unk2a8 > 12; }
    int GetScore() const { return mScore; }
    int GetGrooveState() const { return mGrooveState; }
    int GetLastInTheZone() const { return mLastInTheZone; }
    float GetInTheZoneProgress() const { return mInTheZoneProgress; }
    void SetForceFail(bool b) { mForceFail = b; }
    void SetUnk2a5(bool b) { unk2a5 = b; }
    int ZoneValue() const { return mInTheZone; }

protected:
    RhythmBattlePlayer();

private:
    void AnimateBoxyState(int, bool, bool);
    void UpdateScore(int);

    /** "instruction display" */
    ObjPtr<RndAnimatable> mComboPositionAnim; // 0x8
    /** "instruction display" */
    ObjPtr<RndAnimatable> mComboColorAnim; // 0x1c
    /** "instruction display" */
    ObjPtr<RndAnimatable> mOneXComboEventAnim; // 0x30
    /** "instruction display" */
    ObjPtr<RndAnimatable> mTwoXComboEventAnim; // 0x44
    /** "instruction display" */
    ObjPtr<RndAnimatable> mThreeXComboEventAnim; // 0x58
    /** "instruction display" */
    ObjPtr<RndAnimatable> mFourXComboEventAnim; // 0x6c
    ObjPtr<RndAnimatable> mInTheZoneAnim; // 0x80
    ObjPtr<RndAnimatable> mLoopingMultAnim; // 0x94
    ObjPtr<RndAnimatable> mStaleAnim; // 0xa8
    ObjPtr<RndAnimatable> mInAnim; // 0xbc
    ObjPtr<RndAnimatable> mInAnim2; // 0xd0
    ObjPtr<RndAnimatable> mOutAnim; // 0xe4
    /** "override the world boxydir" */
    ObjPtr<RndDir> mBoxyDir; // 0xf8
    ObjPtr<HamLabel> mComboLabel; // 0x10c
    /** "instruction display" */
    ObjPtr<HamLabel> mScoreLabel; // 0x120
    ObjPtr<Flow> mInTheZoneFlow; // 0x134
    ObjPtr<Flow> mOutTheZoneOkFlow; // 0x148
    ObjPtr<Flow> mOutTheZoneBadFlow; // 0x15c
    ObjPtr<Flow> mSwagJackedFlow; // 0x170
    ObjPtr<HamPhraseMeter> mPhraseMeter; // 0x184
    ObjPtr<TransConstraint> mTransConstraint; // 0x198
    ObjPtr<RndTransformable> mBoxyWaistTrans; // 0x1ac
    ObjPtr<DepthBuffer3D> mBoxyman1; // 0x1c0
    ObjPtr<DepthBuffer3D> mBoxyman2; // 0x1d4
    ObjPtr<ObjectDir> mTextFeedback; // 0x1e8
    ObjPtr<ObjectDir> mMoveFeedback; // 0x1fc
    ObjPtr<RndParticleSys> mStealPartSys; // 0x210
    ObjPtr<RndAnimatable> mStealAnim; // 0x224
    /** "which player is this" */
    int mPlayer; // 0x238
    RhythmBattle *mRhythmBattle; // 0x23c
    bool mActive; // 0x240 - active?
    float mGroove; // 0x244
    float mFreshness; // 0x248
    float mBeatGrooveAccum; // 0x24c
    float mBeatFreshAccum; // 0x250
    float mBeatMoveAccum; // 0x254
    float mAccumBeat; // 0x258
    float mLastPollBeat; // 0x25c
    RhythmBattleScoreState mGrooveState; // 0x260
    RhythmBattleScoreState mLastGrooveState; // 0x264
    int mInTheZone; // 0x268
    int mLastInTheZone; // 0x26c
    float mMeasureGroove; // 0x270
    float mMeasureFresh; // 0x274
    float mMeasureMove; // 0x278
    Symbol mMeasureTrick; // 0x27c
    int mScore; // 0x280
    float mInTheZoneProgress; // 0x284
    bool mGoofySwap; // 0x288
    float mMeasureStartBeat; // 0x28c
    float mMeasureEndBeat; // 0x290
    int mDebugInt; // 0x294
    Symbol mToken; // 0x298
    int mSwitchUpCooldown; // 0x29c
    float mLastFeets; // 0x2a0
    bool mForceFail; // 0x2a4
    bool unk2a5; // 0x2a5
    int unk2a8; // 0x2a8
};
