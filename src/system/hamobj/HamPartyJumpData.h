#pragma once
#include "obj/Object.h"
#include "utl/MemMgr.h"

class HamPartyJumpData : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~HamPartyJumpData();
    OBJ_CLASSNAME(HamPartyJumpData);
    OBJ_SET_TYPE(HamPartyJumpData);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x11)
    NEW_OBJ(HamPartyJumpData)

protected:
    HamPartyJumpData();

    std::vector<std::pair<int, int> > mJumps; // 0x2c
};
