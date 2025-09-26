#pragma once
#include "char/CharPollable.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Does all interpolation for the neck." */
class CharNeckTwist : public CharPollable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CharNeckTwist);
    OBJ_SET_TYPE(CharNeckTwist);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(CharNeckTwist)

protected:
    CharNeckTwist();

    /** "The twist bone, neck must be parent of" */
    ObjPtr<RndTransformable> mTwist; // 0x8
    /** "The head bone, must be descendent of neck" */
    ObjPtr<RndTransformable> mHead; // 0x1c
};
