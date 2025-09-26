#pragma once
#include "char/CharPollable.h"
#include "math/Geo.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Forces the targets to be within a world space bounding box relative to source." */
class CharPosConstraint : public CharPollable {
public:
    // Hmx::Object
    virtual ~CharPosConstraint();
    OBJ_CLASSNAME(CharPosConstraint);
    OBJ_SET_TYPE(CharPosConstraint);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x18)
    NEW_OBJ(CharPosConstraint)

protected:
    CharPosConstraint();

    /** "Bone to be higher than" */
    ObjPtr<RndTransformable> mSrc; // 0x8
    /** "Bones to constrain" */
    ObjPtrList<RndTransformable> mTargets; // 0x1c
    /** "Bounding box, make min > max to ignore that dimension" */
    Box mBox; // 0x30
};
