#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "world/Crowd.h"

class WorldCrowd3DCharHandle : public RndTransformable, public RndDrawable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(WorldCrowd3DCharHandle);
    OBJ_SET_TYPE(WorldCrowd3DCharHandle);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndTransformable
    virtual void UpdatedWorldXfm();
    // RndDrawable
    virtual void DrawShowing();
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(WorldCrowd3DCharHandle)

    void
    Set3DChar(WorldCrowd *, const std::list<WorldCrowd::CharData>::iterator &, int, const Transform &);

protected:
    WorldCrowd3DCharHandle();

    WorldCrowd *mCrowd; // 0x100
    std::list<WorldCrowd::CharData>::iterator mCharItr; // 0x104
    int mCharIdx; // 0x108
};
