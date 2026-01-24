#pragma once
#include "char/CharPollable.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Does all interpolation for the upperarm, assuming
        upperArm, upperTwist1 and 2 are under clavicle. Rotation about x is
        evenly distributed from clavicle->twist1->twist2->upperarm
    Feeds the bones when executed." */
class CharUpperTwist : public CharPollable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CharUpperTwist);
    OBJ_SET_TYPE(CharUpperTwist);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(CharUpperTwist)

protected:
    CharUpperTwist();
    virtual ~CharUpperTwist();

    /** "The upper arm twist1 bone" */
    ObjPtr<RndTransformable> mTwist1; // 0x8
    /** "The upper arm twist2 bone" */
    ObjPtr<RndTransformable> mTwist2; // 0x1c
    /** "The upper arm bone" */
    ObjPtr<RndTransformable> mUpperArm; // 0x30
};
