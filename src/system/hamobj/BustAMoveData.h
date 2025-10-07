#pragma once
#include "obj/Object.h"
#include "utl/MemMgr.h"

class BAMPhrase {
public:
    BAMPhrase() : count(1), bars(4) {}
    int count;
    int bars;
};

class BustAMoveData : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~BustAMoveData();
    OBJ_CLASSNAME(BustAMoveData);
    OBJ_SET_TYPE(BustAMoveData);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(BustAMoveData)
protected:
    BustAMoveData();

    std::vector<BAMPhrase> mPhrases; // 0x2c
};
