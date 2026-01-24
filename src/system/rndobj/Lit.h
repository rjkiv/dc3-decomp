#pragma once
#include "obj/Object.h"
#include "rndobj/CubeTex.h"
#include "rndobj/Trans.h"
#include "math/Color.h"
#include "rndobj/Tex.h"
#include "rndobj/Draw.h"
#include "utl/MemMgr.h"

class RndLight : public RndTransformable {
public:
    enum Type {
        kPoint = 0,
        kDirectional = 1,
        kFakeSpot = 2,
        kFloorSpot = 3,
        kShadowRef = 4,
    };
    enum ProjectedBlend {
    };
    OBJ_CLASSNAME(Light)
    OBJ_SET_TYPE(Light)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual void SetColor(const Hmx::Color &c) { mColorOwner->mColor = c; }
    virtual void SetLightType(Type t) { mType = t; }
    virtual void SetRange(float r) { mRange = r; }
    virtual void SetFalloffStart(float start) { mFalloffStart = start; }

    void SetTopRadius(float rad) { mTopRadius = rad; }
    void SetBotRadius(float rad) { mBotRadius = rad; }
    void SetShadowOverride(ObjPtrList<RndDrawable> *);
    const Hmx::Color &GetColor() const { return mColorOwner->mColor; }
    float Range() const { return mRange; }
    float FalloffStart() const { return mFalloffStart; }
    Type GetType() const { return mType; }
    void SetShowing(bool b) { mShowing = b; }
    float Intensity() const;
    void SetProjectedBlend(int i) { mProjectedBlend = i; }
    // bool GetAnimateFromPreset() const {
    //     return mAnimateColorFromPreset || mAnimatePositionFromPreset
    //         || mAnimateRangeFromPreset;
    // }
    bool Showing() const { return mShowing; }
    // bool AnimatePosFromPreset() const { return mAnimatePositionFromPreset; }
    // bool AnimateRangeFromPreset() const { return mAnimateRangeFromPreset; }

    Transform Projection();

    static const char *TypeToStr(Type);

    OBJ_MEM_OVERLOAD(0x1A);
    NEW_OBJ(RndLight)
    static void Init() { REGISTER_OBJ_FACTORY(RndLight) }

protected:
    RndLight();

    int PackedColor() const;
    void SetPackedColor(int, float);

    Hmx::Color mColor; // 0xc0
    ObjOwnerPtr<RndLight> mColorOwner; // 0xd0
    float mRange; // 0xe4
    float mFalloffStart; // 0xe8
    Type mType; // 0xec
    bool mAnimateColorFromPreset; // 0xf0
    bool mAnimatePositionFromPreset; // 0xf1
    bool mAnimateRangeFromPreset; // 0xf2
    bool mShowing; // 0xf3
    ObjPtr<RndTex> mTexture; // 0xf4
    ObjPtr<RndCubeTex> mCubeTexture; // 0x108
    ObjPtrList<RndDrawable> *mShadowOverride; // 0x11c
    ObjPtrList<RndDrawable> mShadowObjects; // 0x120
    Transform mTextureXfm; // 0x134
    float mTopRadius; // 0x10C
    float mBotRadius; // 0x110
    int mProjectedBlend; // 0x114
};
