#pragma once
#include "math/Color.h"
#include "obj/Object.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"
#include "rndobj/Wind.h"
#include "utl/BinStream.h"

// size 0x9c
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

protected:
    RndFur();

    int mLayers; // 0x2c
    float mThickness; // 0x30
    float mCurvature; // 0x34
    float mShellOut; // 0x38
    float mAlphaFalloff; // 0x3c
    float mStretch; // 0x40
    float mSlide; // 0x44
    float mGravity; // 0x48
    float mFluidity; // 0x4c
    Hmx::Color mRootsTint; // 0x50
    Hmx::Color mEndsTint; // 0x60
    ObjPtr<RndTex> mFurDetail; // 0x70
    float mFurTiling; // 0x84
    ObjPtr<RndWind> mWind; // 0x88
};
