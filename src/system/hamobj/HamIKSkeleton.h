#pragma once
#include "char/CharPollable.h"
#include "hamobj/HamCharacter.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Manages the neutral skeleton for a HamCharacter" */
class HamIKSkeleton : public CharPollable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(HamIKSkeleton);
    OBJ_SET_TYPE(HamIKSkeleton);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(HamIKSkeleton)

    void NeutralLocalPos(RndTransformable *, Vector3 &);
    void NeutralWorldXfm(RndTransformable *, Transform &);

protected:
    HamIKSkeleton();

    virtual void SetName(const char *, ObjectDir *);

    void SetBone(RndTransformable *, RndTransformable *);

    ObjectDir *mNeutralSkelDir; // 0x8
    ObjPtr<HamCharacter> mChar; // 0xc
};
