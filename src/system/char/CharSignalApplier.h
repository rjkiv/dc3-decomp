#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/BinStream.h"

class CharSignalApplier : public CharPollable, public CharWeightable {
public:
    struct BoneOp {};

    // Hmx::Object
    OBJ_CLASSNAME(CharSignalApplier);
    OBJ_SET_TYPE(CharSignalApplier);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // RndPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    NEW_OBJ(CharSignalApplier);

    float unk28;
    float unk2c;
    float unk30;
    bool unk34;
    float unk38;
    float unk3c;
    ObjVector<CharSignalApplier::BoneOp> unk40;

protected:
    CharSignalApplier();
};

bool PropSync(CharSignalApplier::BoneOp &, DataNode &, DataArray *, int, PropOp);
