#pragma once
#include "hamobj/SongUtl.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

/** "Data used for Craze Hollaback" */
class CrazeHollaback : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~CrazeHollaback() {}
    OBJ_CLASSNAME(CrazeHollaback);
    OBJ_SET_TYPE(CrazeHollaback);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x10)
    NEW_OBJ(CrazeHollaback)

protected:
    CrazeHollaback();

    /** "Music loop start and end" */
    Range mMusicRange; // 0x2c
    /** "Playable range of hollaback" */
    Range mPlayRange; // 0x34
};
