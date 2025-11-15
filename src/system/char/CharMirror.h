#pragma once
#include "char/CharBones.h"
#include "char/CharPollable.h"
#include "char/CharServoBone.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

class CharMirror : public CharWeightable, public CharPollable {
public:
    struct MirrorOp { // From RB3 decomp
        void *ptr; // 0x0
        Symbol op; // 0x4
    };
    OBJ_CLASSNAME(CharMirror);
    OBJ_SET_TYPE(CharMirror);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    NEW_OBJ(CharMirror);

    ObjPtr<CharServoBone> mServo; // 0x28
    ObjPtr<CharServoBone> mMirrorServo; // 0x2c
    CharBonesAlloc mBones; // 0x50
    std::vector<MirrorOp> mOps; // 0xdc

    void SetServo(CharServoBone *);
    void SetMirrorServo(CharServoBone *);

protected:
    CharMirror();
    void SyncBones();
};
