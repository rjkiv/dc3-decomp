#pragma once
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Defines the two objects that will be used to determine
    the distance for the texture blend." */
class RndTexBlendController : public Hmx::Object {
public:
    enum BlendState {
        kBlendStateNone = 0x0000,
        kBlendStateNear = 0x0001,
        kBlendStateFar = 0x0002,
        kBlendStateOverride = 0x0003,
    };
    virtual ~RndTexBlendController() {}
    OBJ_CLASSNAME(TexBlendController);
    OBJ_SET_TYPE(TexBlendController);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x15);
    NEW_OBJ(RndTexBlendController)
    REGISTER_OBJ(RndTexBlendController)

    BlendState GetBlendState(float &factor, float scale) const;
    RndTex *GetOverrideMap() const { return mOverrideMap; }
    RndMesh *GetMesh() const { return mMesh; }

protected:
    RndTexBlendController();

    bool IsValid() const;
    bool GetCurrentDistance(float &) const;
    void UpdateReferenceDistance();
    void UpdateMinDistance();
    void UpdateMaxDistance();
    void UpdateAllDistances();

    /** "The mesh object to render to the texture.
        This should be an unskinned mesh with UV coordinates
        that match the source mesh" */
    ObjPtr<RndMesh> mMesh; // 0x2c
    /** "The first object to use as a distance reference" */
    ObjPtr<RndTransformable> mObject1; // 0x40
    /** "The second object to use as a distance reference" */
    ObjPtr<RndTransformable> mObject2; // 0x54
    /** "The base distance used to compute which blending to use" */
    float mReferenceDistance; // 0x68
    /** "The distance where the 'near' texture map will be fully visible" */
    float mMinDistance; // 0x6c
    /** "The distance where the 'far' texture map will be fully visible" */
    float mMaxDistance; // 0x70
    ObjPtr<RndTex> mOverrideMap; // 0x74
};
