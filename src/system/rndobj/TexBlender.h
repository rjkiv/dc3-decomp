#pragma once
#include "obj/Data.h"
#include "rndobj/Draw.h"
#include "rndobj/Tex.h"
#include "rndobj/TexBlendController.h"

/** "Renderable texture used to composite pieces of texture maps
    based on the distance between bones or other animatiable objects" */
class RndTexBlender : public RndDrawable {
public:
    enum TexState {
        kTexState_Base = 0x0001,
        kTexState_Near = 0x0002,
        kTexState_Far = 0x0004,
        kTexState_Override = 0x0008,
    };
    // Hmx::Object
    OBJ_CLASSNAME(TexBlender);
    OBJ_SET_TYPE(TexBlender);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x1B);
    NEW_OBJ(RndTexBlender)
    REGISTER_OBJ(RndTexBlender)

protected:
    RndTexBlender();

    void DrawBlendList(
        const std::vector<std::pair<RndTexBlendController *, float> > &list,
        TexState state
    );
    RndMat *SetupMaterial(RndMat *mat, RndTex *tex);
    DataNode OnGetRenderTextures(DataArray *);

    static RndMat *sBlendMaterial;

    /** "The base texture map" */
    ObjPtr<RndTex> mBaseMap; // 0x40
    /** "The texture map to use when the constraints are closer
        than the default distance" */
    ObjPtr<RndTex> mNearMap; // 0x54
    /** "The texture map to use when the constraints are further
        than the default distance" */
    ObjPtr<RndTex> mFarMap; // 0x68
    /** "The final result output texture" */
    ObjPtr<RndTex> mOutputTexture; // 0x7c
    /** "The list of controller objects used to render
        pieces of a mesh to the output texture" */
    ObjPtrList<RndTexBlendController> mControllerList; // 0x90
    /** "The owner of this texture blend.
        This is used to determine if the texture blend is visible.
        For example, if this texture blend is used in the head object of a character,
        set the owner to be the head object." */
    ObjPtr<RndDrawable> mOwner; // 0xa4
    /** "Global strength of the blending effect for each controller".
        Ranges from 0 to 1. */
    float mFactorScale; // 0xb8
    unsigned int mOutputTexState; // 0xbc
    bool mDirty; // 0xc0
};
