#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "utl/FilePath.h"
#include "rndobj/Tex.h"
#include "rndobj/Mat.h"
#include "utl/MemMgr.h"

class Screenshot : public RndDrawable {
public:
    virtual ~Screenshot();
    OBJ_CLASSNAME(Screenshot);
    OBJ_SET_TYPE(Screenshot);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();

    NEW_OBJ(Screenshot)
    OBJ_MEM_OVERLOAD(0x13)

protected:
    Screenshot();

    /** "File containing the screenshot to display.  This file will only be loaded in
     * Milo, not in game." */
    FilePath mTexPath; // 0x20
    RndTex *mTex; // 0x2c
    RndMat *mMat; // 0x30

private:
    void Sync();
};
