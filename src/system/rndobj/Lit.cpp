#include "rndobj/Lit.h"
#include "Lit.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"

void RndLight::SetShadowOverride(ObjPtrList<RndDrawable> *l) { mShadowOverride = l; }

void RndLight::SetPackedColor(int packed, float scalar) {
    Hmx::Color col;
    col.Unpack(packed);
    Multiply(col, scalar, col);
    SetColor(col);
}

const char *RndLight::TypeToStr(Type t) {
    const char *lightTypes[] = { "Point", "Directional", "Projected", "ShadowRef" };
    MILO_ASSERT(t < DIM(lightTypes), 0x17A);
    return lightTypes[t];
}

void RndLight::Save(BinStream &bs) {
    bs << 0x10;
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mColor << mRange << mType;
    bs << mFalloffStart;
    bs << mAnimateColorFromPreset << mAnimatePositionFromPreset;
    bs << mTopRadius << mBotRadius;
    bs << mTexture;
    bs << mColorOwner;
    bs << mTextureXfm;
    bs << mCubeTexture;
    bs << mShadowObjects;
    bs << mProjectedBlend;
    bs << mAnimateRangeFromPreset;
}

BEGIN_HANDLERS(RndLight)
    HANDLE_ACTION(set_showing, SetShowing(_msg->Int(2)))
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

RndLight::RndLight()
    : mColor(1, 1, 1), mColorOwner(this, this), mRange(1000.0f), mFalloffStart(0),
      mType(kPoint), mAnimateColorFromPreset(1), mAnimatePositionFromPreset(1),
      mAnimateRangeFromPreset(1), mShowing(1), mTexture(this), mCubeTexture(this),
      mShadowOverride(nullptr), mShadowObjects(this, kObjListNoNull), mTopRadius(0),
      mBotRadius(30.0f), mProjectedBlend(0) {
    mTextureXfm.Reset();
}

int RndLight::PackedColor() const {
    Hmx::Color col;
    Multiply(GetColor(), 1.0f / Intensity(), col);
    return col.Pack();
}

float RndLight::Intensity() const {
    Hmx::Color col(GetColor());
    return Max(1.0f, Max(col.red, col.green, col.blue));
}

BEGIN_PROPSYNCS(RndLight)
    SYNC_PROP(animate_color_from_preset, mAnimateColorFromPreset)
    SYNC_PROP(animate_position_from_preset, mAnimatePositionFromPreset)
    SYNC_PROP(animate_range_from_preset, mAnimateRangeFromPreset)
    SYNC_PROP_SET(light_type, mType, SetLightType((Type)_val.Int()))
    SYNC_PROP_SET(range, mRange, SetRange(_val.Float()))
    SYNC_PROP_SET(falloff_start, mFalloffStart, SetFalloffStart(_val.Float()))
    SYNC_PROP_SET(color, PackedColor(), SetPackedColor(_val.Int(), Intensity()))
    SYNC_PROP_SET(intensity, Intensity(), SetPackedColor(PackedColor(), _val.Float()))
    SYNC_PROP_SET(topradius, mTopRadius, SetTopRadius(_val.Float()))
    SYNC_PROP_SET(botradius, mBotRadius, SetBotRadius(_val.Float()))
    SYNC_PROP(color_owner, mColorOwner)
    SYNC_PROP(texture, mTexture)
    SYNC_PROP(cube_texture, mCubeTexture)
    SYNC_PROP(texture_xfm, mTextureXfm)
    SYNC_PROP_SET(projected_blend, mProjectedBlend, SetProjectedBlend(_val.Int()))
    SYNC_PROP(shadow_objects, mShadowObjects)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_LOADS(RndLight)
    LOAD_REVS(bs)
    ASSERT_REVS(0x10, 0)
    if (d.rev > 3)
        LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    bs >> mColor;
    if (d.rev < 2) {
        Hmx::Color col1, col2;
        bs >> col1 >> col2;
    }
    if (d.rev < 3) {
        int i, j;
        bs >> i >> j;
    }
    bs >> mRange;
    if (d.rev < 3) {
        int i, j, k;
        bs >> i >> j >> k;
    }
    if (d.rev > 0) {
        int count;
        bs >> count;
        if (d.rev < 0xE) {
            if (count > 1)
                count--;
        }
        mType = (Type)count;
    }
    if (d.rev > 0xB) {
        bs >> mFalloffStart;
    }
    if (d.rev > 4) {
        bs >> mAnimateColorFromPreset >> mAnimatePositionFromPreset;
    }
    if (d.rev > 6) {
        bs >> mTopRadius >> mBotRadius;
        if (d.rev < 0xE) {
            int i, j;
            bs >> i >> j;
        }
    }
    if (d.rev > 7) {
        bs >> mTexture;
        if (d.rev == 9) {
            ObjPtrList<RndDrawable> drawList(this);
            bs >> drawList;
        } else if (d.rev == 8) {
            ObjPtr<RndDrawable> drawPtr(this);
            bs >> drawPtr;
        }
    }
    if (d.rev > 10) {
        bs >> mColorOwner;
        if (!mColorOwner)
            mColorOwner = this;
    }
    if (d.rev > 0xC)
        bs >> mTextureXfm;
    if (d.rev > 0xD) {
        bs >> mCubeTexture;
    }
    if (d.rev > 0xE) {
        bs >> mShadowObjects;
        bs >> mProjectedBlend;
    }
    if (d.rev > 0xF)
        bs >> mAnimateRangeFromPreset;
    else
        mAnimateRangeFromPreset = mAnimateColorFromPreset;
END_LOADS
