#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "utl/MemMgr.h"

/** "Forces the PostProc" */
class PostProcer : public RndDrawable {
public:
    // Hmx::Object
    virtual ~PostProcer() {}
    OBJ_CLASSNAME(PostProcer);
    OBJ_SET_TYPE(PostProcer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x11)
    NEW_OBJ(PostProcer)

protected:
    PostProcer();
};
