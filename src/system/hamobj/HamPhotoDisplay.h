#pragma once
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Mesh.h"
#include "utl/MemMgr.h"

class HamPhotoDisplay : public RndDir {
public:
    HamPhotoDisplay();
    // Hmx::Object
    OBJ_CLASSNAME(HamPhotoDisplay);
    OBJ_SET_TYPE(HamPhotoDisplay);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x13)
    NEW_OBJ(HamPhotoDisplay)
    static void Init();

private:
    void DrawPhotoMesh(RndMesh *, int);

protected:
    ObjPtr<RndMesh> mMesh1; // 0x1fc
    ObjPtr<RndMesh> mMesh2; // 0x210
    int mIndex1; // 0x224
    int mIndex2; // 0x228
};
