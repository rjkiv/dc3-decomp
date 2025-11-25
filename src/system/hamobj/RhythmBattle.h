#pragma once
#include "gesture/ArchiveSkeleton.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/HamLabel.h"
#include "hamobj/RhythmBattlePlayer.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "stl/_vector.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

class RhythmBattlePlayer;

class RhythmBattle : public RndPollable {
public:
    // Hmx::Object
    virtual ~RhythmBattle();
    OBJ_CLASSNAME(RhythmBattle);
    OBJ_SET_TYPE(RhythmBattle);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();

    OBJ_MEM_OVERLOAD(0x23)
    NEW_OBJ(RhythmBattle)

    void End();
    void ResetCombo();
    void Begin();

    ObjPtr<HamLabel> mCommandLabel; // 0x8
    ObjPtr<HamLabel> unk1c;
    ObjPtr<RhythmBattlePlayer> mPlayerOne; // 0x30
    ObjPtr<RhythmBattlePlayer> mPlayerTwo; // 0x44
    ObjPtr<RndTransformable> unk58;
    ObjPtr<RndAnimatable> unk6c;
    ObjPtr<RndAnimatable> unk80;
    ObjPtr<RndAnimatable> unk94;
    ObjPtr<RndAnimatable> unka8;
    ObjPtr<RndAnimatable> unkbc;
    ObjPtr<RndAnimatable> unkd0;
    ObjPtr<RndAnimatable> unke4;
    bool unkf8;
    bool unkf9;
    bool unkfa;
    bool mActive; // 0xfb
    bool unkfc;
    bool unkfd;
    bool unkfe;
    bool unkff;
    bool unk100;
    bool unk101;
    bool unk102;
    float unk104;
    float unk108;
    float unk10c;
    float unk110;
    float unk114;
    float unk118;
    int unk11c;
    float unk120;
    int unk124;
    int unk128;
    Symbol unk12c;
    FreestyleMoveRecorder *unk130;
    std::vector<ArchiveSkeleton> unk134;
    int unk140;
    u32 unk144;
    int unk148;
    int unk14c;
    std::vector<Symbol> unk150;

protected:
    RhythmBattle();

private:
    bool GetGoofy() const;
    Symbol GetLeader() const;
    void OnPause();
    void OnUnpause();
    void CheckIsFinale();
    void PlayTanClip(int, bool);
    void PlayMindControlVO(Symbol);
    void UpdateMindControl();
    void UpdateFinaleVO(int &);
    void QueueFinaleVO(Symbol);
    void OnReset();
    void OnBeat();
};

void SetJump(int, int);
void ClearJump();
