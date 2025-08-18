#include "rndobj/BaseMaterial.h"
#include "Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"

#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

BaseMaterial *gDefaultMat;

namespace {
    bool IsMat(BaseMaterial *mat) {
        if (mat && mat->ClassName() == "Mat") {
            return true;
        }
        return false;
    }
}

void BaseMaterial::SetDefaultMat(BaseMaterial *mat) {
    MILO_ASSERT(!gDefaultMat, 0x55);
    gDefaultMat = mat;
}

const DataNode *BaseMaterial::GetDefaultPropVal(Symbol s) {
    const DataNode *node = gDefaultMat->Property(s, true);
    MILO_ASSERT(node, 0x129);
    return node;
}

void MatPerfSettings::Save(BinStream &bs) const {
    bs << mRecvProjLights;
    bs << mPS3ForceTrilinear;
    bs << mRecvPointCubeTex;
}

void MatPerfSettings::LoadOld(BinStreamRev &bs) {
    bs >> mRecvProjLights;
    bs >> mPS3ForceTrilinear;
    if (bs.mRev > 0x41)
        bs >> mRecvPointCubeTex;
}

void MatPerfSettings::Load(BinStream &bs) {
    bs >> mRecvProjLights;
    bs >> mPS3ForceTrilinear;
    bs >> mRecvPointCubeTex;
}

void BaseMaterial::Save(BinStream &bs) {
    bs << 8;
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mBlend;
    bs << mColor << mUseEnviron << mPrelit;
    bs << mZMode;
    bs << mAlphaCut << mAlphaThreshold << mAlphaWrite;
    bs << mTexGen;
    bs << mTexWrap;
    bs << mTexXfm;
    bs << mDiffuseTex << mNextPass;
    bs << mIntensify;
    bs << mCull;
    bs << mEmissiveMultiplier;
    bs << mSpecularRGB << mNormalMap;
    bs << mEmissiveMap << mSpecularMap;
    bs << mEnvironMap << mEnvironMapFalloff << mEnvironMapSpecMask;
    bs << mPerPixelLit;
    bs << mStencilMode;
    bs << mFur << mDeNormal << mAnisotropy;
    bs << mNormDetailTiling << mNormDetailStrength << mNormDetailMap;
    bs << mPointLights << mFog << mFadeout << mColorAdjust;
    bs << mRimRGB << mRimMap << mRimLightUnder;
    bs << mScreenAligned;
    bs << mShaderVariation;
    bs << mSpecular2RGB;
    mPerfSettings.Save(bs);
    bs << mRefractEnabled;
    bs << mRefractStrength;
    bs << mRefractNormalMap;
    bs << mBloomMultiplier;
    bs << mNeverFitToSpline;
    bs << mAllowDistortionEffects;
    bs << mShockwaveMult;
    bs << mWorldProjectionTiling;
    bs << mWorldProjectionStartBlend;
    bs << mWorldProjectionEndBlend;
    bs << mDiffuseTex2;
    bs << mForceAlphaWrite;
}

bool BaseMaterial::PropValDifferent(Symbol s, BaseMaterial *base) {
    if (!base)
        base = gDefaultMat;
    MILO_ASSERT(base, 0x133);
    if (s == "tex_xfm") {
        return base->mTexXfm != mTexXfm;
    } else {
        const DataNode *node = Property(s, true);
        MILO_ASSERT(node, 0x13C);
        DataNode var(*node);
        DataNode othervar(*Property(s, true));
        if (s == "shader_combos") {
            return var > othervar;
        } else {
            return var != othervar;
        }
    }
}

DataNode BaseMaterial::OnIsDefaultPropVal(const DataArray *a) {
    static Symbol tex_xfm("tex_xfm");
    Symbol sym = a->Sym(2);
    bool ret;
    if (sym == tex_xfm) {
        Transform tf;
        tf.Reset();
        ret = tf == mTexXfm;
    } else {
        const DataNode *val = GetDefaultPropVal(sym);
        MILO_ASSERT(val, 0x1AA);
        DataNode var(*val);
        val = Property(sym, true);
        MILO_ASSERT(val, 0x1AF);
        ret = val->Equal(var, nullptr, true);
    }
    return ret;
}

BaseMaterial::BaseMaterial()
    : mUseEnviron(true), mPrelit(false), mDiffuseTex(this), mDiffuseTex2(this),
      mIntensify(false), mTexWrap(kTexWrapRepeat), mTexGen(kTexGenNone),
      mBlend(kBlendSrc), mZMode(kZModeNormal), mAlphaCut(false), mAlphaWrite(false),
      mForceAlphaWrite(false), mAlphaThreshold(0), mNextPass(this), mPointLights(false),
      mFog(false), mFadeout(false), mColorAdjust(false), mPerPixelLit(false),
      mEmissiveMultiplier(1), mEmissiveMap(this), mNormalMap(this), mDeNormal(0),
      mNormDetailMap(this), mNormDetailTiling(1), mNormDetailStrength(0),
      mSpecularRGB(0, 0, 0, 10), mSpecular2RGB(0, 0, 0, 10), mSpecularMap(this),
      mAnisotropy(0), mRimRGB(0, 0, 0, 10), mRimMap(this), mRimLightUnder(false),
      mEnvironMap(this), mEnvironMapFalloff(false), mEnvironMapSpecMask(false),
      mRefractEnabled(false), mRefractStrength(0), mRefractNormalMap(this),
      mShaderVariation(kShaderVariationNone), mFur(this), mNeverFitToSpline(false),
      mAllowDistortionEffects(true), mShockwaveMult(1), mWorldProjectionTiling(0.125),
      mWorldProjectionStartBlend(0.8), mWorldProjectionEndBlend(0.9),
      mScreenAligned(false), mCull(kCullRegular), mStencilMode(kStencilIgnore),
      mBloomMultiplier(1) {
    mTexXfm.Reset();
}

BEGIN_COPYS(BaseMaterial)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(BaseMaterial)
    BEGIN_COPYING_MEMBERS
        if (ty == kCopyFromMax) {
            if (mDiffuseTex != c->mDiffuseTex) {
                COPY_MEMBER(mDiffuseTex)
            }
            if (mDiffuseTex2 != c->mDiffuseTex2) {
                COPY_MEMBER(mDiffuseTex2)
            }
        } else {
            COPY_MEMBER(mZMode)
            COPY_MEMBER(mStencilMode)
            COPY_MEMBER(mBlend)
            COPY_MEMBER(mColor)
            COPY_MEMBER(mPrelit)
            COPY_MEMBER(mUseEnviron)
            COPY_MEMBER(mAlphaCut)
            COPY_MEMBER(mAlphaThreshold)
            COPY_MEMBER(mAlphaWrite)
            COPY_MEMBER(mForceAlphaWrite)
            COPY_MEMBER(mTexGen)
            COPY_MEMBER(mTexWrap)
            COPY_MEMBER(mTexXfm)
            COPY_MEMBER(mDiffuseTex)
            COPY_MEMBER(mDiffuseTex2)
            COPY_MEMBER(mNextPass)
            COPY_MEMBER(mCull)
            COPY_MEMBER(mEmissiveMultiplier)
            COPY_MEMBER(mSpecularRGB)
            COPY_MEMBER(mSpecular2RGB)
            COPY_MEMBER(mNormalMap)
            COPY_MEMBER(mEmissiveMap)
            COPY_MEMBER(mSpecularMap)
            COPY_MEMBER(mEnvironMap)
            COPY_MEMBER(mEnvironMapFalloff)
            COPY_MEMBER(mEnvironMapSpecMask)
            COPY_MEMBER(mIntensify)
            COPY_MEMBER(mPerPixelLit)
            COPY_MEMBER(mFur)
            COPY_MEMBER(mDeNormal)
            COPY_MEMBER(mAnisotropy)
            COPY_MEMBER(mNormDetailTiling)
            COPY_MEMBER(mNormDetailStrength)
            COPY_MEMBER(mNormDetailMap)
            COPY_MEMBER(mPointLights)
            COPY_MEMBER(mFog)
            COPY_MEMBER(mFadeout)
            COPY_MEMBER(mColorAdjust)
            COPY_MEMBER(mRimRGB)
            COPY_MEMBER(mRimMap)
            COPY_MEMBER(mRimLightUnder)
            COPY_MEMBER(mScreenAligned)
            COPY_MEMBER(mShaderVariation)
            COPY_MEMBER(mPerfSettings)
            COPY_MEMBER(mRefractEnabled)
            COPY_MEMBER(mRefractStrength)
            COPY_MEMBER(mRefractNormalMap)
            COPY_MEMBER(mBloomMultiplier)
            COPY_MEMBER(mNeverFitToSpline)
            COPY_MEMBER(mAllowDistortionEffects)
            COPY_MEMBER(mShockwaveMult)
            COPY_MEMBER(mWorldProjectionTiling)
            COPY_MEMBER(mWorldProjectionStartBlend)
            COPY_MEMBER(mWorldProjectionEndBlend)
        }
    END_COPYING_MEMBERS
END_COPYS

BaseMaterial::Blend CheckBlendMode(BaseMaterial::Blend b, BaseMaterial *) { return b; }

// enum BaseMaterial::Blend __cdecl CheckBlendMode(enum BaseMaterial::Blend, class
// BaseMaterial *)

BEGIN_LOADS(BaseMaterial)
    LOAD_REVS(bs)
    ASSERT_REVS(8, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bsrev >> BinStreamEnum<Blend>(mBlend);
    mBlend = CheckBlendMode(mBlend, this);
    bs >> mColor >> mUseEnviron >> mPrelit;
    bsrev >> BinStreamEnum<ZMode>(mZMode);
    bsrev >> mAlphaCut;
    bs >> mAlphaThreshold;
    bsrev >> mAlphaWrite;
    bsrev >> BinStreamEnum<TexGen>(mTexGen) >> BinStreamEnum<TexWrap>(mTexWrap);
END_LOADS

bool BaseMaterial::IsNextPass(BaseMaterial *m) {
    for (BaseMaterial *it = this; it != nullptr; it = it->NextPass()) {
        if (it == m)
            return true;
    }
    return false;
}

DataNode BaseMaterial::OnAllowedNextPass(const DataArray *a) {
    int matCount = 0;
    for (ObjDirItr<BaseMaterial> it(Dir(), true); it != nullptr; ++it) {
        if (IsMat(it))
            matCount++;
    }
    matCount += 2;
    DataArrayPtr ptr(new DataArray(matCount));
    int idx = 0;
    ptr->Node(idx++) = NULL_OBJ;

    if (mNextPass) {
        ptr->Node(idx++) = mNextPass.Ptr();
    }

    for (ObjDirItr<BaseMaterial> it(Dir(), true); it != nullptr; ++it) {
        if (IsMat(it)) {
            if (!IsNextPass(it)) {
                ptr->Node(idx++) = &*it;
            }
        }
    }
    ptr->Resize(idx);
    return ptr;
}

DataNode BaseMaterial::OnAllowedNormalMap(const DataArray *a) {
    return GetNormalMapTextures(Dir());
}

BEGIN_HANDLERS(BaseMaterial)
    HANDLE(allowed_next_pass, OnAllowedNextPass)
    HANDLE(allowed_normal_map, OnAllowedNormalMap)
    HANDLE(is_default, OnIsDefaultPropVal)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(BaseMaterial)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS
