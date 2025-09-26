#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Rotate a bone to point towards targets" */
class CharBoneTwist : public CharPollable, public CharWeightable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CharBoneTwist);
    OBJ_SET_TYPE(CharBoneTwist);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(CharBoneTwist)

protected:
    CharBoneTwist();

    /** "Bone to move" */
    ObjPtr<RndTransformable> mBone; // 0x28
    /** "Targets to average to point bone at" */
    ObjPtrList<RndTransformable> mTargets; // 0x3c
};
