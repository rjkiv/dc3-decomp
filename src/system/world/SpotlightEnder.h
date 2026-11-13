#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "utl/MemMgr.h"

/** "Object that ends spotlights, must put after the last spotlight
    and before anything that would render using those spotlights" */
class SpotlightEnder : public RndDrawable {
public:
    // Hmx::Object
    virtual ~SpotlightEnder() {}
    OBJ_CLASSNAME(SpotlightEnder);
    OBJ_SET_TYPE(SpotlightEnder);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(SpotlightEnder)
    REGISTER_OBJ(SpotlightEnder)

protected:
    SpotlightEnder();
};
