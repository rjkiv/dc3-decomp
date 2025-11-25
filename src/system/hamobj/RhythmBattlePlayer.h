#pragma once

#include "flow/Flow.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/RhythmBattle.h"
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

enum RhythmBattleJackState {
};

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

    ObjPtr<RndAnimatable> unk8;
    ObjPtr<RndAnimatable> unk1c;
    ObjPtr<RndAnimatable> unk30;
    ObjPtr<RndAnimatable> unk44;
    ObjPtr<RndAnimatable> unk58;
    ObjPtr<RndAnimatable> unk6c;
    ObjPtr<RndAnimatable> unk80;
    ObjPtr<RndAnimatable> unk94;
    ObjPtr<RndAnimatable> unka8;
    ObjPtr<RndAnimatable> unkbc;
    ObjPtr<RndAnimatable> unkd0;
    ObjPtr<RndAnimatable> unke4;
    ObjPtr<RndDir> unkf8;
    ObjPtr<HamLabel> unk10c;
    ObjPtr<HamLabel> unk120;
    ObjPtr<Flow> unk134;
    ObjPtr<Flow> unk148;
    ObjPtr<Flow> unk15c;
    ObjPtr<Flow> unk170;
    ObjPtr<HamPhraseMeter> unk184;
    ObjPtr<TransConstraint> unk198;
    ObjPtr<RndTransformable> unk1ac;
    ObjPtr<HamMove> unk1c0; // change to DepthBuffer3D once class made
    ObjPtr<HamMove> unk1d4; // change to DepthBuffer3D once class made
    ObjPtr<ObjectDir> unk1e8;
    ObjPtr<ObjectDir> unk1fc;
    ObjPtr<RndParticleSys> unk210;
    ObjPtr<RndAnimatable> unk224;
    int unk238;
    int unk23c;
    bool unk240;
    float unk244;
    float unk248;
    float unk24c;
    float unk250;
    u32 unk254;
    float unk258;
    float unk25c;
    int unk260;
    int unk264;
    int mInTheZone; // 0x268
    int unk26c;
    float unk270;
    float unk274;
    u32 unk278;
    Symbol unk27c;
    int unk280;
    float unk284;
    bool unk288;
    float unk28c;
    float unk290;
    int unk294;
    Symbol unk298;
    int unk29c;
    u32 unk2a0;
    bool unk2a4;
    bool unk2a5;
    int unk2a8;

protected:
    RhythmBattlePlayer();

private:
    void AnimateBoxyState(int, bool, bool);
    void UpdateScore(int);
};
