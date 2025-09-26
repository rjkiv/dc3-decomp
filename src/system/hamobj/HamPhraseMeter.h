#pragma once
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "utl/MemMgr.h"

/** "Custom component for move feedback progress" */
class HamPhraseMeter : public RndDir {
public:
    HamPhraseMeter();
    // Hmx::Object
    OBJ_CLASSNAME(HamPhraseMeter);
    OBJ_SET_TYPE(HamPhraseMeter);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x10)
    NEW_OBJ(HamPhraseMeter)

    void SetRatingFrac(float, float);

protected:
    /** "what do animate" */
    ObjPtr<RndAnimatable> mAnim; // 0x1fc
    /** "Current rating frac (for testing)". Ranges from 0 to 1. */
    float mRatingFrac; // 0x210
    /** "Current rating symbol (for testing)".
        Options are (move_perfect move_awesome move_ok move_bad) */
    Symbol mRating; // 0x214
    /** "Desired animation speed (frames per beat)" */
    float mDesiredFPB; // 0x218
    /** "Value of the first frame of a perfect rating" */
    float mFirstPerfectFrame; // 0x21c
    float unk220; // 0x220
    float unk224; // 0x224
    /** "Player index this phrase meter is for". Is either 0 or 1. */
    int mPlayerIndex; // 0x228
};
