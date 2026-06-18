#include "rndobj/Lit.h"
#include "Lit.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"

RndLight::RndLight()
    : mColor(1, 1, 1), mColorOwner(this, this), mRange(1000.0f), mFalloffStart(0),
      mType(kPoint), mAnimateColorFromPreset(1), mAnimatePositionFromPreset(1),
      mAnimateRangeFromPreset(1), mShowing(1), mTexture(this), mCubeTexture(this),
      mShadowOverride(nullptr), mShadowObjects(this, kObjListNoNull), mTopRadius(0),
      mBotRadius(30.0f), mProjectedBlend(0) {
    mTextureXfm.Reset();
}

bool RndLight::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mColorOwner == from) {
        if (mColorOwner == this) {
            mColorOwner = this;
        } else {
            RndLight *lit = dynamic_cast<RndLight *>(to);
            if (lit) {
                mColorOwner = lit->mColorOwner.Ptr();
            } else {
                mColorOwner = this;
            }
        }
        return true;
    } else {
        return RndTransformable::Replace(from, to);
    }
}

BEGIN_HANDLERS(RndLight)
    HANDLE_ACTION(set_showing, SetShowing(_msg->Int(2)))
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

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

BEGIN_SAVES(RndLight)
    SAVE_REVS(0x10, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mColor << mRange << mType;
    bs << mFalloffStart;
    bs << mAnimateColorFromPreset;
    bs << mAnimatePositionFromPreset;
    bs << mTopRadius << mBotRadius;
    bs << mTexture;
    bs << mColorOwner;
    bs << mTextureXfm;
    bs << mCubeTexture;
    bs << mShadowObjects;
    bs << mProjectedBlend;
    bs << mAnimateRangeFromPreset;
END_SAVES

BEGIN_COPYS(RndLight)
    CREATE_COPY_AS(RndLight, l)
    MILO_ASSERT(l, 0xC4);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_MEMBER_FROM(l, mColor)
    COPY_MEMBER_FROM(l, mType)
    COPY_MEMBER_FROM(l, mAnimateColorFromPreset)
    COPY_MEMBER_FROM(l, mAnimatePositionFromPreset)
    COPY_MEMBER_FROM(l, mAnimateRangeFromPreset)
    if (ty != kCopyFromMax) {
        COPY_MEMBER_FROM(l, mRange)
    }
    COPY_MEMBER_FROM(l, mFalloffStart)
    COPY_MEMBER_FROM(l, mTopRadius)
    COPY_MEMBER_FROM(l, mBotRadius)
    COPY_MEMBER_FROM(l, mTexture)
    COPY_MEMBER_FROM(l, mCubeTexture)
    COPY_MEMBER_FROM(l, mShadowOverride)
    COPY_MEMBER_FROM(l, mShadowObjects)
    COPY_MEMBER_FROM(l, mProjectedBlend)
    if (ty == kCopyShallow || (ty == kCopyFromMax && l->mColorOwner != l)) {
        mColorOwner = l->mColorOwner.Ptr();
    } else {
        mColorOwner = this;
        COPY_MEMBER_FROM(l, mColor)
    }
END_COPYS

INIT_REVS(0x10, 0)

BEGIN_LOADS(RndLight)
    LOAD_REVS(bs)
    ASSERT_REVS(0x10, 0)
    if (d.rev > 3)
        LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    d >> mColor;
    if (d.rev < 2) {
        Hmx::Color col1, col2;
        d.stream >> col1 >> col2;
    }
    if (d.rev < 3) {
        int i, j;
        d >> i >> j;
    }
    d >> mRange;
    if (d.rev < 3) {
        int i, j, k;
        d >> i >> j >> k;
    }
    if (d.rev > 0) {
        int count;
        d >> count;
        if (d.rev < 0xE) {
            if (count > 1)
                count--;
        }
        mType = (Type)count;
    }
    if (d.rev > 0xB) {
        d >> mFalloffStart;
    }
    if (d.rev <= 4 || d.rev >= 5) {
        if (d.rev > 5) {
            d >> mAnimateColorFromPreset;
            d >> mAnimatePositionFromPreset;
        }
    } else {
        bool b;
        d >> b;
        mAnimateColorFromPreset = b;
        mAnimatePositionFromPreset = b;
    }
    if (d.rev > 6) {
        d >> mTopRadius >> mBotRadius;
        if (d.rev < 0xE) {
            int i, j;
            d >> i >> j;
        }
    }
    if (d.rev > 7) {
        d >> mTexture;
        if (d.rev == 9) {
            ObjPtrList<RndDrawable> drawList(this);
            d >> drawList;
        } else if (d.rev == 8) {
            ObjPtr<RndDrawable> drawPtr(this);
            d >> drawPtr;
        }
    }
    if (d.rev > 10) {
        d >> mColorOwner;
        if (!mColorOwner)
            mColorOwner = this;
    }
    if (d.rev > 0xC)
        d >> mTextureXfm;
    if (d.rev > 0xD) {
        d >> mCubeTexture;
    }
    if (d.rev > 0xE) {
        d >> mShadowObjects;
        d >> mProjectedBlend;
    }
    if (d.rev > 0xF)
        d >> mAnimateRangeFromPreset;
    else
        mAnimateRangeFromPreset = mAnimateColorFromPreset;
END_LOADS

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

int RndLight::PackedColor() const {
    Hmx::Color col;
    Multiply(GetColor(), 1.0f / Intensity(), col);
    return col.Pack();
}

float RndLight::Intensity() const {
    Hmx::Color col(GetColor());
    return Max(1.0f, Max(col.red, col.green, col.blue));
}

Transform RndLight::Projection() {
    if (mRange == 0) {
        Transform out;
        out.Reset();
        return out;
    } else {
        Transform out;
        Vector3 mx = WorldXfm().m.x;
        Vector3 mz;
        Negate(WorldXfm().m.z, mz);
        Vector3 my = WorldXfm().m.y;
        Vector3 v = WorldXfm().v;
        float radSlope = (mBotRadius - mTopRadius) / mRange;
        Scale(my, radSlope, my);

        out.m.x.x = mx.x;
        out.m.x.y = mz.x;
        out.m.x.z = my.x;

        out.m.y.x = mx.y;
        out.m.y.y = mz.y;
        out.m.y.z = my.y;

        out.m.z.x = mx.z;
        out.m.z.y = mz.z;
        out.m.z.z = my.z;

        out.v.x = -Dot(mx, v);
        out.v.y = -Dot(mz, v);
        out.v.z = mTopRadius - Dot(my, v);

        Multiply(out, mTextureXfm, out);
        static Transform sXfm(
            Hmx::Matrix3(0.5f, 0, 0, 0, 0.5f, 0, 0.5f, 0.5f, 1), Vector3(0, 0, 0)
        );
        Multiply(out, sXfm, out);
        return out;
    }
}
