#include "rndobj/Fur.h"

#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

void RndFur::Save(BinStream &bs) {
    bs << 3;
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mLayers << mThickness << mCurvature;
    bs << mShellOut << mAlphaFalloff;
    bs << mStretch << mSlide << mGravity << mFluidity;
    bs << mRootsTint << mEndsTint;
    bs << mFurDetail << mFurTiling;
    bs << mWind;
}

RndFur::RndFur()
    : mLayers(12), mThickness(3), mCurvature(2), mShellOut(1), mAlphaFalloff(0.5),
      mStretch(1), mSlide(1), mGravity(1), mFluidity(0.25), mRootsTint(0, 0, 0),
      mFurDetail(this), mFurTiling(1), mWind(this) {}

BEGIN_COPYS(RndFur)
    CREATE_COPY_AS(RndFur, m)
    MILO_ASSERT(m, 0x7D);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_MEMBER_FROM(m, mLayers)
    COPY_MEMBER_FROM(m, mThickness)
    COPY_MEMBER_FROM(m, mCurvature)
    COPY_MEMBER_FROM(m, mShellOut)
    COPY_MEMBER_FROM(m, mAlphaFalloff)
    COPY_MEMBER_FROM(m, mStretch)
    COPY_MEMBER_FROM(m, mSlide)
    COPY_MEMBER_FROM(m, mGravity)
    COPY_MEMBER_FROM(m, mFluidity)
    COPY_MEMBER_FROM(m, mRootsTint)
    COPY_MEMBER_FROM(m, mEndsTint)
    COPY_MEMBER_FROM(m, mFurDetail)
    COPY_MEMBER_FROM(m, mFurTiling)
    COPY_MEMBER_FROM(m, mWind)
END_COPYS

BEGIN_HANDLERS(RndFur)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndFur)
    SYNC_PROP(layers, mLayers)
    SYNC_PROP(thickness, mThickness)
    SYNC_PROP(curvature, mCurvature)
    SYNC_PROP(shell_out, mShellOut)
    SYNC_PROP(alpha_falloff, mAlphaFalloff)
    SYNC_PROP(stretch, mStretch)
    SYNC_PROP(slide, mSlide)
    SYNC_PROP(gravity, mGravity)
    SYNC_PROP(fluidity, mFluidity)
    SYNC_PROP(roots_tint, mRootsTint)
    SYNC_PROP(ends_tint, mEndsTint)
    SYNC_PROP(fur_detail, mFurDetail)
    SYNC_PROP(fur_tiling, mFurTiling)
    SYNC_PROP(wind, mWind)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

bool RndFur::LoadOld(BinStreamRev &bs) {
    bool ret;
    bs >> ret;
    if (ret || bs.mRev < 0x20) {
        bs >> mLayers;
        bs >> mThickness;
        bs >> mCurvature;
        bs >> mStretch;
        bs >> mSlide;
        bs >> mGravity;
        bs >> mFluidity;
        bs >> mRootsTint >> mEndsTint;
        if (bs.mRev > 0x1E) {
            bs >> mFurDetail >> mFurTiling;
        }
        if (bs.mRev < 0x24) {
            Vector3 v;
            float f;
            bs >> v >> f;
        }
    }
    return ret;
}

BEGIN_LOADS(RndFur)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mLayers;
    bs >> mThickness;
    bs >> mCurvature;
    if (gRev > 1) {
        bs >> mShellOut;
        bs >> mAlphaFalloff;
    }
    bs >> mStretch;
    bs >> mSlide;
    bs >> mGravity;
    bs >> mFluidity;
    bs >> mRootsTint >> mEndsTint;
    bs >> mFurDetail;
    bs >> mFurTiling;
    if (gRev > 2) {
        bs >> mWind;
    }
END_LOADS
