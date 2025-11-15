#pragma once
#include "char/CharPollable.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "stl/_vector.h"

class CharBlendBone : public CharPollable {
public:
    struct ConstraintSystem {
    public:
        ConstraintSystem();
        ConstraintSystem(Hmx::Object *);

        /** "object to constrain" */
        ObjPtr<RndTransformable> mTarget; // 0x0
        /** "influence value, from 0 to 1" */
        float mWeight; // 0xc
    };

    // Hmx::Object
    OBJ_CLASSNAME(CharBlendBone);
    OBJ_SET_TYPE(CharBlendBone);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // RndPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    NEW_OBJ(CharBlendBone);

    ObjVector<ConstraintSystem> mTargets; // 0x8
    ObjPtr<RndTransformable> mSrc1; // 0x18
    ObjPtr<RndTransformable> mSrc2; // 0x2c
    bool mTransX; // 0x40
    bool mTransY; // 0x41
    bool mTransZ; // 0x42
    bool mRotation; // 0x43
    bool mSetLocal; // 0x44

protected:
    CharBlendBone();
};

BinStream &operator<<(BinStream &, CharBlendBone::ConstraintSystem &);
