#pragma once
#include "gesture/ArchiveSkeleton.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/HamLabel.h"
#include "hamobj/RhythmBattlePlayer.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include <vector>

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
    bool InFullKTB() const { return mFullOnKtB; }
    bool IsPaused() const { return mPaused; }

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
    ObjPtr<HamLabel> mCommandLabel2; // 0x1c
    /** "player 0 object" */
    ObjPtr<RhythmBattlePlayer> mPlayer0; // 0x30
    /** "player 1 object" */
    ObjPtr<RhythmBattlePlayer> mPlayer1; // 0x44
    ObjPtr<RndTransformable> mBoxyLeadHead; // 0x58
    ObjPtr<RndAnimatable> mInAnim; // 0x6c
    ObjPtr<RndAnimatable> mOutAnim; // 0x80
    ObjPtr<RndAnimatable> mTugAnim; // 0x94
    ObjPtr<RndAnimatable> mJackAnim1R; // 0xa8
    ObjPtr<RndAnimatable> mJackAnim1L; // 0xbc
    ObjPtr<RndAnimatable> mJackAnim2R; // 0xd0
    ObjPtr<RndAnimatable> mJackAnim2L; // 0xe4
    bool mGoofySide; // 0xf8
    /** "is this keep the beat, or just groove tech experience" */
    bool mFullOnKtB; // 0xf9
    bool mFinale; // 0xfa
    bool mActive; // 0xfb
    bool mAnimIn; // 0xfc
    bool mIntroVO; // 0xfd
    bool mMidwayVO; // 0xfe
    bool mAlmostOverVO; // 0xff
    bool mIntroOver; // 0x100
    bool mOutroBegun; // 0x101
    bool mPaused; // 0x102
    float mStartBeat; // 0x104
    float mEndBeat; // 0x108
    /** Mind control mini-game after Tan's army storm and hijach DCI HQ. */
    float mMindControlProgress; // 0x10c
    float mLastMindControlVOTime; // 0x110
    float mMidwayBeat;
    float mAlmostOverBeat;
    int mBeatCount; // 0x11c
    float mSynchronizedRating; // 0x120
    RhythmBattleJackState mJackState; // 0x124 - RhythmBattleJackState
    int mWarnJackCounter; // 0x128
    Symbol mLastLeader; // 0x12c
    FreestyleMoveRecorder *mRecorder;
    std::vector<ArchiveSkeleton> mRecorderPair;
    int mStealCooldown; // 0x140
    int mLastBeat; // 0x144
    int mBeatsToNextScriptedStage; // 0x148
    int mScriptedStageNum; // 0x14c
    std::vector<Symbol> mFinaleVOQueue; // 0x150
};

void SetJump(int, int);
void ClearJump();
