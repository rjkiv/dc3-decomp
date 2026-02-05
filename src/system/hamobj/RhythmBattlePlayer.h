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
    bool Unk2a8Check() const { return unk2a5 && unk2a8 > 12; }
    int Unk280() const { return unk280; }
    int Unk260() const { return unk260; }

protected:
    RhythmBattlePlayer();

private:
    void AnimateBoxyState(int, bool, bool);
    void UpdateScore(int);

    /** "instruction display" */
    ObjPtr<RndAnimatable> mComboPosAnim; // 0x8
    /** "instruction display" */
    ObjPtr<RndAnimatable> mComboColorAnim; // 0x1c
    /** "instruction display" */
    ObjPtr<RndAnimatable> mResetComboAnim; // 0x30
    /** "instruction display" */
    ObjPtr<RndAnimatable> m2xMultAnim; // 0x44
    /** "instruction display" */
    ObjPtr<RndAnimatable> m3xMultAnim; // 0x58
    /** "instruction display" */
    ObjPtr<RndAnimatable> m4xMultAnim; // 0x6c
    ObjPtr<RndAnimatable> mRhythmBattleAnim; // 0x80
    ObjPtr<RndAnimatable> unk94; // 0x94
    ObjPtr<RndAnimatable> mBattleMeterStaleAnim; // 0xa8
    ObjPtr<RndAnimatable> mBattleMeterInAnim; // 0xbc
    ObjPtr<RndAnimatable> mShowScoreAnim; // 0xd0
    ObjPtr<RndAnimatable> mBattleMeterOutAnim; // 0xe4
    /** "override the world boxydir" */
    ObjPtr<RndDir> mBoxyDir; // 0xf8
    ObjPtr<HamLabel> unk10c;
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
    ObjPtr<RndParticleSys> mStealPart; // 0x210
    ObjPtr<RndAnimatable> mStealAnim; // 0x224
    /** "which player is this" */
    int mPlayer; // 0x238
    RhythmBattle *mRhythmBattle; // 0x23c
    bool unk240; // 0x240 - active?
    float unk244;
    float unk248;
    float unk24c;
    float unk250;
    float unk254;
    float unk258;
    float unk25c;
    int unk260;
    int unk264;
    int mInTheZone; // 0x268
    int unk26c;
    float unk270;
    float unk274;
    float unk278;
    Symbol unk27c;
    int unk280;
    float unk284;
    bool unk288;
    float unk28c;
    float unk290;
    int unk294;
    Symbol unk298;
    int unk29c;
    float unk2a0;
    bool unk2a4;
    bool unk2a5;
    int unk2a8;
};
