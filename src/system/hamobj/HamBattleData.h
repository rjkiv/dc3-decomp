#pragma once
#include "hamobj/HamCamShot.h"
#include "hamobj/SongUtl.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

class BattleStep {
public:
    BattleStep() : mState("normal"), mPlayers(kHamPlayerBoth), mNonplayAction("idle") {}
    BattleStep &operator=(const BattleStep &step) {
        mState = step.mState;
        mPlayers = step.mPlayers;
        mMusicRange = step.mMusicRange;
        mPlayRange = step.mPlayRange;
        mCam = step.mCam;
        mNonplayAction = step.mNonplayAction;
        return *this;
    }

    /** "What's going on during this section of the battle".
        Options are: normal, minigame */
    Symbol mState; // 0x0
    /** "Which players are involved with this section" */
    HamPlayerFlags mPlayers; // 0x4
    /** "Music loop start and end" */
    Range mMusicRange; // 0x8
    /** "Playable range of the section" */
    Range mPlayRange; // 0x10
    /** "Which camera cut to use for this section".
        Options are: '', Area1_NEAR, Area1_FAR, Area1_MOVEMENT,
        Area2_NEAR, Area2_FAR, Area2_MOVEMENT, DC_PLAYER_FREESTYLE */
    Symbol mCam; // 0x18
    /** "What the non-dancer is doing". Options are: idle, dance, hide */
    Symbol mNonplayAction; // 0x1c
};

class HamBattleData : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~HamBattleData();
    OBJ_CLASSNAME(HamBattleData);
    OBJ_SET_TYPE(HamBattleData);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x22)
    NEW_OBJ(HamBattleData)

protected:
    HamBattleData();

    /** "Steps for the dance battle" */
    std::vector<BattleStep> mSteps; // 0x2c
};
