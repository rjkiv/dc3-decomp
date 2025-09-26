#pragma once
#include "char/CharPollable.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Does all interpolation for the forearm. Assumes:
        -foretwist1 and forearm are under upperarm.
        -foretwist2 is under foretwist1 and that hand is under forearm.
        -on the left hand offset rotation is usually 90 on the left, and -90 on the right.
    Feeds the bones when executed." */
class CharForeTwist : public CharPollable, public RndHighlightable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CharForeTwist);
    OBJ_SET_TYPE(CharForeTwist);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x1D)
    NEW_OBJ(CharForeTwist)

protected:
    CharForeTwist();

    // RndHighlightable
    virtual void Highlight() {}

    /** "The hand bone" */
    ObjPtr<RndTransformable> mHand; // 0x10
    /** "The twist2 bone" */
    ObjPtr<RndTransformable> mTwist2; // 0x24
    /** "Usually 180 for right hand, 0 for left hand" */
    float mOffset; // 0x38
    /** "Biases the angle before dividing by 3,
        typically 45 on the left hand, -45 on the right." */
    float mBias; // 0x3c
};
