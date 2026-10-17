#include "rndobj/BaseMaterial.h"
#include "Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"

#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

BaseMaterial *gDefaultMat;

namespace {
    bool IsMat(BaseMaterial *mat) { return mat && mat->ClassName() == "Mat"; }
}

#pragma region MatPerfSettings

void MatPerfSettings::Save(BinStream &bs) const {
    bs << mRecvProjLights;
    bs << mPS3ForceTrilinear;
    bs << mRecvPointCubeTex;
}

void MatPerfSettings::LoadOld(BinStreamRev &bs) {
    bs >> mRecvProjLights;
    bs >> mPS3ForceTrilinear;
    if (bs.rev > 0x41)
        bs >> mRecvPointCubeTex;
}

void MatPerfSettings::Load(BinStream &bs) {
    bs >> mRecvProjLights;
    bs >> mPS3ForceTrilinear;
    bs >> mRecvPointCubeTex;
}

#pragma endregion
#pragma region BaseMaterial

BaseMaterial::BaseMaterial()
    : mColor(1, 1, 1), mUseEnviron(true), mPrelit(false), mDiffuseTex(this),
      mDiffuseTex2(this), mIntensify(false), mTexWrap(BaseMaterial::kRepeat),
      mTexGen(kNone), mBlend(kBlendSrc), mZMode(kNormal), mAlphaCut(false),
      mAlphaWrite(false), mForceAlphaWrite(false), mAlphaThresh(0), mNextPass(this),
      mPointLights(false), mFog(false), mFadeout(false), mColorXfm(false),
      mPerPixelLit(false), mEmissiveMultiplier(1), mEmissiveMap(this), mNormalMap(this),
      mDeNormal(0), mNormDetail(this), mNormDetailTile(1), mNormDetailStrength(0),
      mSpecular(0, 0, 0, 10), mSpecularSecondary(0, 0, 0, 10), mSpecularMap(this),
      mAnisotropy(0), mRimColor(0, 0, 0, 10), mRimMap(this), mRimLightUnder(false),
      mEnvironMap(this), mEnvironMapFalloff(false), mEnvironMapSpecMask(false),
      mRefractEnabled(false), mRefractStrength(0), mRefractNormalMap(this),
      mShaderVariation(kShaderVariation_None), mFur(this), mNeverFitToSpline(false),
      mAllowDistortionEffects(true), mShockwaveMult(1), mWorldProjectionTile(0.125),
      mWorldProjectionStartBlend(0.8), mWorldProjectionEndBlend(0.9),
      mScreenAligned(false), mCull(kCullCW), mStencilMode(kIgnore), mBloomMultiplier(1) {
    mTexXfm.Reset();
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

BEGIN_SAVES(BaseMaterial)
    SAVE_REVS(8, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mBlend << mColor << mUseEnviron << mPrelit;
    bs << mZMode << mAlphaCut << mAlphaThresh << mAlphaWrite;
    bs << mTexGen << mTexWrap << mTexXfm << mDiffuseTex << mNextPass;
    bs << mIntensify << mCull << mEmissiveMultiplier;
    bs << mSpecular << mNormalMap;
    bs << mEmissiveMap << mSpecularMap;
    bs << mEnvironMap << mEnvironMapFalloff << mEnvironMapSpecMask;
    bs << mPerPixelLit << mStencilMode;
    bs << mFur << mDeNormal << mAnisotropy;
    bs << mNormDetailTile << mNormDetailStrength << mNormDetail;
    bs << mPointLights << mFog << mFadeout << mColorXfm;
    bs << mRimColor << mRimMap << mRimLightUnder;
    bs << mScreenAligned << mShaderVariation << mSpecularSecondary;
    mPerfSettings.Save(bs);
    bs << mRefractEnabled << mRefractStrength << mRefractNormalMap;
    bs << mBloomMultiplier << mNeverFitToSpline;
    bs << mAllowDistortionEffects << mShockwaveMult;
    bs << mWorldProjectionTile;
    bs << mWorldProjectionStartBlend;
    bs << mWorldProjectionEndBlend;
    bs << mDiffuseTex2;
    bs << mForceAlphaWrite;
END_SAVES

BEGIN_COPYS(BaseMaterial)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(BaseMaterial)
    BEGIN_COPYING_MEMBERS
        if (ty == kCopyFromMax) {
            if (!mDiffuseTex != !c->mDiffuseTex) {
                COPY_MEMBER(mDiffuseTex)
            }
            if (!mDiffuseTex2 != !c->mDiffuseTex2) {
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
            COPY_MEMBER(mAlphaThresh)
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
            COPY_MEMBER(mSpecular)
            COPY_MEMBER(mSpecularSecondary)
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
            COPY_MEMBER(mNormDetailTile)
            COPY_MEMBER(mNormDetailStrength)
            COPY_MEMBER(mNormDetail)
            COPY_MEMBER(mPointLights)
            COPY_MEMBER(mFog)
            COPY_MEMBER(mFadeout)
            COPY_MEMBER(mColorXfm)
            COPY_MEMBER(mRimColor)
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
            COPY_MEMBER(mWorldProjectionTile)
            COPY_MEMBER(mWorldProjectionStartBlend)
            COPY_MEMBER(mWorldProjectionEndBlend)
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(8, 0)

BEGIN_LOADS(BaseMaterial)
    LOAD_REVS(bs)
    ASSERT_REVS(8, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> (int &)mBlend;
    mBlend = CheckBlendMode(mBlend, this);
    d >> mColor >> mUseEnviron >> mPrelit;
    d >> (int &)mZMode;
    d >> mAlphaCut >> mAlphaThresh >> mAlphaWrite;
    d >> (int &)mTexGen >> (int &)mTexWrap >> mTexXfm >> mDiffuseTex >> mNextPass;
    d >> mIntensify;
    if (d.rev < 3) {
        bool cull;
        d >> cull;
        mCull = (CullType)(cull != 0);
    } else {
        d >> (int &)mCull;
    }
    d >> mEmissiveMultiplier;
    d >> mSpecular >> mNormalMap;
    d >> mEmissiveMap >> mSpecularMap;
    d >> mEnvironMap >> mEnvironMapFalloff >> mEnvironMapSpecMask;
    d >> mPerPixelLit >> (int &)mStencilMode;
    d >> mFur >> mDeNormal >> mAnisotropy;
    d >> mNormDetailTile >> mNormDetailStrength >> mNormDetail;
    d >> mPointLights >> mFog >> mFadeout >> mColorXfm;
    d >> mRimColor >> mRimMap >> mRimLightUnder;
    d >> mScreenAligned;
    d >> (int &)mShaderVariation;
    d >> mSpecularSecondary;
    mPerfSettings.Load(d);
    d >> mRefractEnabled;
    d >> mRefractStrength;
    d >> mRefractNormalMap;
    if (d.rev > 1) {
        d >> mBloomMultiplier;
    }
    if (d.rev > 3) {
        d >> mNeverFitToSpline;
        if (d.rev < 5) {
            bool b1;
            d >> b1;
            d >> b1;
        }
        if (d.rev >= 6) {
            d >> mAllowDistortionEffects;
            d >> mShockwaveMult;
        }
    }
    if (d.rev > 6) {
        d >> mWorldProjectionTile;
        d >> mWorldProjectionStartBlend;
        d >> mWorldProjectionEndBlend;
        d >> mDiffuseTex2;
    }
    if (d.rev > 7) {
        d >> mForceAlphaWrite;
    }
END_LOADS

void BaseMaterial::SetDefaultMat(BaseMaterial *mat) {
    MILO_ASSERT(!gDefaultMat, 0x55);
    gDefaultMat = mat;
}

const DataNode *BaseMaterial::GetDefaultPropVal(Symbol s) {
    const DataNode *node = gDefaultMat->Property(s, true);
    MILO_ASSERT(node, 0x129);
    return node;
}

bool BaseMaterial::PropValDifferent(Symbol s, BaseMaterial *base) {
    // absolute hogwash but hey it matches
    if (!base && (base = gDefaultMat, !gDefaultMat)) {
        MILO_ASSERT(base, 0x133);
    }
    if (s == "tex_xfm") {
        return base->mTexXfm != mTexXfm;
    } else {
        const DataNode *node = Property(s);
        MILO_ASSERT(node, 0x13C);
        DataNode var(*node);
        DataNode othervar(*base->Property(s));
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

NOINLINE BaseMaterial::Blend CheckBlendMode(BaseMaterial::Blend b, BaseMaterial *) {
    return b;
}

bool BaseMaterial::IsNextPass(BaseMaterial *m) {
    for (BaseMaterial *it = this; it != nullptr; it = it->mNextPass) {
        if (it == m) {
            return true;
        }
    }
    return false;
}

DataNode BaseMaterial::OnAllowedNextPass(const DataArray *a) {
    int matCount = 0;
    for (ObjDirItr<BaseMaterial> it(Dir(), true); it != nullptr; ++it) {
        if (IsMat(it)) {
            matCount++;
        }
    }
    matCount += 2;
    DataArrayPtr ptr(new DataArray(matCount));
    int idx = 0;
    ptr->Node(idx++) = NULL_OBJ;

    if (mNextPass) {
        ptr->Node(idx++) = mNextPass.Ptr();
    }

    for (ObjDirItr<BaseMaterial> it(Dir(), true); it != nullptr; ++it) {
        if (IsMat(it) && !IsNextPass(it)) {
            ptr->Node(idx++) = &*it;
        }
    }
    ptr->Resize(idx);
    return ptr;
}

DataNode BaseMaterial::OnAllowedNormalMap(const DataArray *a) {
    return GetNormalMapTextures(Dir());
}

#pragma endregion
