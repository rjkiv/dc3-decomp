#pragma once
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"

class RndMovie : public RndAnimatable {
public:
    // Hmx::Object
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Movie);
    OBJ_SET_TYPE(Movie);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndAnimatable
    virtual bool Loop() { return mLoop; }
    virtual void SetFile(const FilePath &file, bool stream) {
        mFile = file;
        mStream = stream;
    }
    virtual void SetTex(RndTex *tex);

    OBJ_MEM_OVERLOAD(0x18)
    NEW_OBJ(RndMovie)
    REGISTER_OBJ(RndMovie)

protected:
    RndMovie();

    /** "File - extension is automatically converted to platform" */
    FilePath mFile; // 0x10
    /** "Stream the file for this movie?" */
    bool mStream; // 0x18
    /** "Loop this movie?" */
    bool mLoop; // 0x19
    /** "Texture to animate." */
    ObjOwnerPtr<RndTex> mTex; // 0x1c
};
