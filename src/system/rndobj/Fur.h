#pragma once
#include "math/Color.h"
#include "obj/Object.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"
#include "rndobj/Wind.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

// size 0x9c
/** "Parameters for fur shading, to be set on a material" */
class RndFur : public Hmx::Object {
public:
    OBJ_CLASSNAME(Fur);
    OBJ_SET_TYPE(Fur);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    bool LoadOld(BinStreamRev &);

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(RndFur)
    static void Init() { REGISTER_OBJ_FACTORY(RndFur); }

    float Fluidity() const { return mFluidity; }
    RndWind *Wind() const { return mWind; }

protected:
    RndFur();

    /** "Number of passes" */
    int mLayers; // 0x2c
    /** "Length of fur" */
    float mThickness; // 0x30
    /** "Curvature exponent". Ranges from 0 to 3. */
    float mCurvature; // 0x34
    /** "Bunch shells towards surface". Ranges from 0 to 1. */
    float mShellOut; // 0x38
    /** "Bunch opacity towards surface". Ranges from 0 to 1. */
    float mAlphaFalloff; // 0x3c
    /** "Maximum stretch" */
    float mStretch; // 0x40
    /** "Maximum lateral motion" */
    float mSlide; // 0x44
    /** "Strength of gravity". Ranges from 0 to 1. */
    float mGravity; // 0x48
    /** "Langor of motion". Ranges from 0 to 1. */
    float mFluidity; // 0x4c
    /** "Tint at hair roots" */
    Hmx::Color mRootsTint; // 0x50
    /** "Tint at hair ends" */
    Hmx::Color mEndsTint; // 0x60
    /** "Detail map for finer fur.  Only the alpha channel is used." */
    ObjPtr<RndTex> mFurDetail; // 0x70
    /** "Tiling for fur detail map.
        UVs of fur_detail are multiplied by this value."
        Ranges from 2.0e-2 to 100.
    */
    float mFurTiling; // 0x84
    /** "Wind Object, if set, blows on the fur." */
    ObjPtr<RndWind> mWind; // 0x88
};
