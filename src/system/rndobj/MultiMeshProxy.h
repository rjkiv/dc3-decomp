#pragma once
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "rndobj/MultiMesh.h"
#include "utl/MemMgr.h"
#include <list>

class RndMultiMeshProxy : public RndTransformable, public RndDrawable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(MultiMeshProxy);
    OBJ_SET_TYPE(MultiMeshProxy);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    virtual void Highlight() { RndDrawable::Highlight(); }

    OBJ_MEM_OVERLOAD(0x14);
    NEW_OBJ(RndMultiMeshProxy)
    REGISTER_OBJ(RndMultiMeshProxy)
    RndMultiMesh *MultiMesh() const { return mMultiMesh; }
    void
    SetMultiMesh(RndMultiMesh *, const std::list<RndMultiMesh::Instance>::iterator &);
    std::list<RndMultiMesh::Instance>::iterator Index() const { return mIndex; }

protected:
    RndMultiMeshProxy();

    // RndTransformable
    virtual void UpdatedWorldXfm();

    ObjPtr<RndMultiMesh> mMultiMesh; // 0x100
    std::list<RndMultiMesh::Instance>::iterator mIndex; // 0x114
};
