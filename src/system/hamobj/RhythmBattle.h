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

/** "Competition between two RhythmBattlePlayers." */
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
    bool CanTrick(Symbol);
    bool InFullKTB() const { return mFullKTB; }
    bool Unk102() const { return unk102; }

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

    /** "instruction display" */
    ObjPtr<HamLabel> mCommandLabel; // 0x8
    ObjPtr<HamLabel> mIntroLine2Label; // 0x1c
    /** "player 0 object" */
    ObjPtr<RhythmBattlePlayer> mPlayerOne; // 0x30
    /** "player 1 object" */
    ObjPtr<RhythmBattlePlayer> mPlayerTwo; // 0x44
    ObjPtr<RndTransformable> mBoxyLeadHeadTrans; // 0x58
    ObjPtr<RndAnimatable> unk6c;
    ObjPtr<RndAnimatable> unk80;
    ObjPtr<RndAnimatable> unk94;
    ObjPtr<RndAnimatable> mSwagJack1BarP2ToP1Anim; // 0xa8
    ObjPtr<RndAnimatable> mSwagJack1BarP1ToP2Anim; // 0xbc
    ObjPtr<RndAnimatable> mSwagJack2BarP2ToP1Anim; // 0xd0
    ObjPtr<RndAnimatable> mSwagJack2BarP1ToP2Anim; // 0xe4
    bool mGoofy; // 0xf8
    /** "is this keep the beat, or just groove tech experience" */
    bool mFullKTB; // 0xf9
    bool mFinale; // 0xfa
    bool mActive; // 0xfb
    bool unkfc;
    bool unkfd;
    bool unkfe;
    bool unkff;
    bool unk100;
    bool unk101;
    bool unk102;
    float mStartBeat; // 0x104
    float mEndBeat; // 0x108
    float unk10c;
    float unk110;
    float unk114;
    float unk118;
    int unk11c;
    float unk120;
    int unk124;
    int unk128;
    Symbol mLeader; // 0x12c
    FreestyleMoveRecorder *unk130;
    std::vector<ArchiveSkeleton> unk134;
    int unk140;
    int unk144;
    int unk148;
    int unk14c;
    std::vector<Symbol> unk150;
};

void SetJump(int, int);
void ClearJump();
