#include "rndobj/MetaMaterial.h"
#include "BaseMaterial.h"
#include "MetaMaterial.h"
#include "obj/Data.h"

#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

namespace {
    // size 0x10
    struct PropAction {
        PropAction(MatProp p, MatPropEditAction e, const char *n, int x)
            : prop(p), edit_action(e), name(n), unkc(x) {}
        MatProp prop;
        MatPropEditAction edit_action;
        const char *name;
        int unkc;
    };

    PropAction gDefaultProps[] = {
        PropAction(kMatPropColor, kPropDefault, "color", 1),
        PropAction(kMatPropAlpha, kPropDefault, "alpha", 1),
        PropAction(kMatPropUseEnviron, kPropDefault, "use_environ", 4),
        PropAction(kMatPropPrelit, kPropDefault, "prelit", 2),
        PropAction(kMatPropDiffuseTex, kPropDefault, "diffuse_tex", 1),
        PropAction(kMatPropDiffuseTex2, kPropDefault, "diffuse_tex2", 1),
        PropAction(kMatPropIntensify, kPropDefault, "intensify", 2),
        PropAction(kMatPropTexWrap, kPropDefault, "tex_wrap", 1),
        PropAction(kMatPropTexGen, kPropDefault, "tex_gen", 4),
        PropAction(kMatPropTexXfm, kPropDefault, "tex_xfm", 1),
        PropAction(kMatPropBlend, kPropDefault, "blend", 1),
        PropAction(kMatPropZMode, kPropDefault, "z_mode", 1),
        PropAction(kMatPropAlphaCut, kPropDefault, "alpha_cut", 2),
        PropAction(kMatPropAlphaWrite, kPropDefault, "alpha_write", 2),
        PropAction(kMatPropNextPass, kPropDefault, "next_pass", 1),
        PropAction(kMatPropPointLights, kPropDefault, "point_lights", 3),
        PropAction(kMatPropFog, kPropDefault, "fog", 2),
        PropAction(kMatPropFadeout, kPropDefault, "fade_out", 3),
        PropAction(kMatPropColorAdjust, kPropDefault, "color_adjust", 2),
        PropAction(kMatPropRecvProjLights, kPropDefault, "recv_proj_lights", 2),
        PropAction(kMatPropRecvPointCubeTex, kPropDefault, "recv_point_cube_tex", 2),
        PropAction(kMatPropPS3ForceTrilinear, kPropDefault, "ps3_force_trilinear", 1),
        PropAction(kMatPropPerPixelLit, kPropDefault, "per_pixel_lit", 2),
        PropAction(kMatPropEmissiveMultiplier, kPropDefault, "emissive_multiplier", 1),
        PropAction(kMatPropEmissiveMap, kPropDefault, "emissive_map", 1),
        PropAction(kMatPropNormalMap, kPropDefault, "normal_map", 1),
        PropAction(kMatPropDeNormal, kPropDefault, "de_normal", 1),
        PropAction(kMatPropNormDetailMap, kPropDefault, "norm_detail_map", 1),
        PropAction(kMatPropNormDetailStrength, kPropDefault, "norm_detail_strength", 1),
        PropAction(kMatPropNormDetailTiling, kPropDefault, "norm_detail_tiling", 1),
        PropAction(kMatPropSpecularRGB, kPropDefault, "specular_rgb", 2),
        PropAction(kMatPropSpecularPower, kPropDefault, "specular_power", 1),
        PropAction(kMatPropSpecularMap, kPropDefault, "specular_map", 1),
        PropAction(kMatPropAnisotropy, kPropDefault, "anisotropy", 2),
        PropAction(kMatPropRimRGB, kPropDefault, "rim_rgb", 2),
        PropAction(kMatPropRimPower, kPropDefault, "rim_power", 1),
        PropAction(kMatPropRimMap, kPropDefault, "rim_map", 1),
        PropAction(kMatPropRimLightUnder, kPropDefault, "rim_light_under", 2),
        PropAction(kMatPropEnvironMap, kPropDefault, "environ_map", 1),
        PropAction(kMatPropEnvironMapFalloff, kPropDefault, "environ_map_falloff", 2),
        PropAction(kMatPropEnvironMapSpecMask, kPropDefault, "environ_map_specmask", 2),
        PropAction(kMatPropRefractEnabled, kPropDefault, "refract_enabled", 2),
        PropAction(kMatPropRefractStrength, kPropDefault, "refract_strength", 1),
        PropAction(kMatPropRefractNormalMap, kPropDefault, "refract_normal_map", 1),
        PropAction(kMatPropShaderVariation, kPropDefault, "shader_variation", 3),
        PropAction(kMatPropSpecular2RGB, kPropDefault, "specular2_rgb", 1),
        PropAction(kMatPropSpecular2Power, kPropDefault, "specular2_power", 1),
        PropAction(kMatPropFur, kPropDefault, "fur", 2),
        PropAction(kMatPropScreenAligned, kPropDefault, "screen_aligned", 2),
        PropAction(kMatPropCull, kPropDefault, "cull", 1),
        PropAction(kMatPropStencilMode, kPropDefault, "stencil_mode", 1),
        PropAction(kMatPropBloomMultiplier, kPropDefault, "bloom_multiplier", 1),
        PropAction(kMatPropNeverFitToSpline, kPropDefault, "never_fit_to_spline", 1),
        PropAction(
            kMatPropAllowDistortionEffects, kPropDefault, "allow_distortion_effects", 1
        ),
        PropAction(kMatPropShockwaveMult, kPropDefault, "shockwave_mult", 1),
        PropAction(
            kMatPropWorldProjectionTiling, kPropDefault, "world_projection_tiling", 1
        ),
        PropAction(
            kMatPropWorldProjectionStartBlend, kPropDefault, "world_projection_tiling", 1
        ),
        PropAction(
            kMatPropWorldProjectionEndBlend, kPropDefault, "world_projection_tiling", 1
        ),
        PropAction(kMatPropForceAlphaWrite, kPropDefault, "force_alpha_write", 1)
    };
    Symbol GetPropName(MatProp p) {
        for (int i = 0; i < DIM(gDefaultProps); i++) {
            if (p == gDefaultProps[i].prop) {
                return gDefaultProps[i].name;
            }
        }
        MILO_FAIL("PropAction %d not found in MetaMaterial", p);
        return gNullStr;
    }
}

int MetaMaterial::CalcApproxNumShaders() {
    int num = 1;
    for (int i = 0; i < DIM(gDefaultProps); i++) {
        String str(gDefaultProps[i].name);
        str += "_edit_action";
        const DataNode *node = Property(str.c_str(), true);
        MILO_ASSERT(node && node->Type() == kDataInt, 0xFB);
        if (node->Int() == 2) {
            num *= gDefaultProps[i].unkc;
        }
    }
    return num;
}

void MetaMaterial::SetEditAction(MatProp propNum, MatPropEditAction action) {
    MILO_ASSERT(propNum < kMatPropMax, 0x8F);
    if (action == kPropDefault || action == kPropForce) {
        Symbol propName = GetPropName(propNum);
        const DataNode *val = GetDefaultPropVal(propName);
        if (action == kPropDefault) {
            if (propNum == kMatPropTexXfm) {
                mTexXfm.Reset();
            } else {
                SetProperty(propName, *val);
                if (propNum == kMatPropAlphaCut) {
                    static Symbol alpha_threshold("alpha_threshold");
                    SetProperty(alpha_threshold, *val);
                }
            }
        } else {
            DataNode var(*val);
            const DataNode *node = Property(propName, true);
            MILO_ASSERT(node, 0xAC);
            if (propNum == kMatPropTexXfm) {
                Transform xfm;
                xfm.Reset();
                action = (MatPropEditAction)(mTexXfm == xfm);
            } else {
                action = (MatPropEditAction)var.Equal(*node, nullptr, true);
            }
        }
    }
    mMatPropEditActions[propNum] = action;
}

void MetaMaterial::Save(BinStream &bs) {
    bs << 2;
    SAVE_SUPERCLASS(BaseMaterial)
    bs << mMatPropEditActions;
}

BEGIN_HANDLERS(MetaMaterial)
    HANDLE_SUPERCLASS(BaseMaterial)
END_HANDLERS

#define SYNC_MAT_PROP_EDIT_ACTION(s, idx)                                                \
    SYNC_PROP_SET(                                                                       \
        s,                                                                               \
        mMatPropEditActions[idx],                                                        \
        SetEditAction((MatProp)idx, (MatPropEditAction)_val.Int())                       \
    )

#define SYNC_MAT_PROP(s, member, idx)                                                    \
    SYNC_PROP_SET(                                                                       \
        s##_edit_action,                                                                 \
        mMatPropEditActions[idx],                                                        \
        SetEditAction((MatProp)idx, (MatPropEditAction)_val.Int())                       \
    )                                                                                    \
    SYNC_PROP(s, member)

BEGIN_PROPSYNCS(MetaMaterial)
    SYNC_PROP_SET(shader_combos, CalcApproxNumShaders(), )
    SYNC_MAT_PROP(color, mColor, kMatPropColor)
    SYNC_MAT_PROP(alpha, mColor.alpha, kMatPropAlpha)
    SYNC_MAT_PROP(use_environ, mUseEnviron, kMatPropUseEnviron)
    SYNC_MAT_PROP(prelit, mPrelit, kMatPropPrelit)
    SYNC_MAT_PROP(diffuse_tex, mDiffuseTex, kMatPropDiffuseTex)
    SYNC_MAT_PROP(diffuse_tex2, mDiffuseTex2, kMatPropDiffuseTex2)
    SYNC_MAT_PROP(intensify, mIntensify, kMatPropIntensify)
    SYNC_MAT_PROP(tex_wrap, (int &)mTexWrap, kMatPropTexWrap)
    SYNC_MAT_PROP(tex_gen, (int &)mTexGen, kMatPropTexGen)
    SYNC_MAT_PROP(tex_xfm, mTexXfm, kMatPropTexXfm)
    SYNC_MAT_PROP(blend, (int &)mBlend, kMatPropBlend)
    SYNC_MAT_PROP(z_mode, (int &)mZMode, kMatPropZMode)
    SYNC_MAT_PROP(alpha_cut, mAlphaCut, kMatPropAlphaCut)
    SYNC_PROP(alpha_threshold, mAlphaThreshold)
    SYNC_MAT_PROP(alpha_write, mAlphaWrite, kMatPropAlphaWrite)
    SYNC_MAT_PROP_EDIT_ACTION(force_alpha_write_edit_action, kMatPropForceAlphaWrite)
    SYNC_PROP(alpha_write, mForceAlphaWrite) // yes, the same symbol was used twice
    SYNC_MAT_PROP(next_pass, mNextPass, kMatPropNextPass)
    SYNC_MAT_PROP(point_lights, mPointLights, kMatPropPointLights)
    SYNC_MAT_PROP(fog, mFog, kMatPropFog)
    SYNC_MAT_PROP(fade_out, mFadeout, kMatPropFadeout)
    SYNC_MAT_PROP(color_adjust, mColorAdjust, kMatPropColorAdjust)
    SYNC_MAT_PROP_EDIT_ACTION(recv_proj_lights_edit_action, kMatPropRecvProjLights)
    SYNC_PROP_SET(
        recv_proj_lights,
        mPerfSettings.mRecvProjLights,
        mPerfSettings.mRecvProjLights = _val.Int() > 0
    )
    SYNC_MAT_PROP_EDIT_ACTION(recv_point_cube_tex_edit_action, kMatPropRecvPointCubeTex)
    SYNC_PROP_SET(
        recv_point_cube_tex,
        mPerfSettings.mRecvPointCubeTex,
        mPerfSettings.mRecvPointCubeTex = _val.Int() > 0
    )
    SYNC_MAT_PROP_EDIT_ACTION(ps3_force_trilinear_edit_action, kMatPropPS3ForceTrilinear)
    SYNC_PROP_SET(
        ps3_force_trilinear,
        mPerfSettings.mPS3ForceTrilinear,
        mPerfSettings.mPS3ForceTrilinear = _val.Int() > 0
    )
    SYNC_MAT_PROP(per_pixel_lit, mPerPixelLit, kMatPropPerPixelLit)
    SYNC_MAT_PROP(emissive_multiplier, mEmissiveMultiplier, kMatPropEmissiveMultiplier)
    SYNC_MAT_PROP(emissive_map, mEmissiveMap, kMatPropEmissiveMap)
    SYNC_MAT_PROP(normal_map, mNormalMap, kMatPropNormalMap)
    SYNC_MAT_PROP(de_normal, mDeNormal, kMatPropDeNormal)
    SYNC_MAT_PROP(norm_detail_map, mNormDetailMap, kMatPropNormDetailMap)
    SYNC_MAT_PROP(norm_detail_strength, mNormDetailStrength, kMatPropNormDetailStrength)
    SYNC_MAT_PROP(norm_detail_tiling, mNormDetailTiling, kMatPropNormDetailTiling)
    SYNC_MAT_PROP(specular_rgb, mSpecularRGB, kMatPropSpecularRGB)
    SYNC_MAT_PROP(specular_power, mSpecularRGB.alpha, kMatPropSpecularPower)
    SYNC_MAT_PROP(specular_map, mSpecularMap, kMatPropSpecularMap)
    SYNC_MAT_PROP(anisotropy, mAnisotropy, kMatPropAnisotropy)
    SYNC_MAT_PROP(rim_rgb, mRimRGB, kMatPropRimRGB)
    SYNC_MAT_PROP(rim_power, mRimRGB.alpha, kMatPropRimPower)
    SYNC_MAT_PROP(rim_map, mRimMap, kMatPropRimMap)
    SYNC_MAT_PROP(rim_light_under, mRimLightUnder, kMatPropRimLightUnder)
    SYNC_MAT_PROP(environ_map, mEnvironMap, kMatPropEnvironMap)
    SYNC_MAT_PROP(environ_map_falloff, mEnvironMapFalloff, kMatPropEnvironMapFalloff)
    SYNC_MAT_PROP(environ_map_specmask, mEnvironMapSpecMask, kMatPropEnvironMapSpecMask)
    SYNC_MAT_PROP(refract_enabled, mRefractEnabled, kMatPropRefractEnabled)
    SYNC_MAT_PROP(refract_strength, mRefractStrength, kMatPropRefractStrength)
    SYNC_MAT_PROP(refract_normal_map, mRefractNormalMap, kMatPropRefractNormalMap)
    SYNC_MAT_PROP(shader_variation, (int &)mShaderVariation, kMatPropShaderVariation)
    SYNC_MAT_PROP(specular2_rgb, mSpecular2RGB, kMatPropSpecular2RGB)
    SYNC_MAT_PROP(specular2_power, mSpecular2RGB.alpha, kMatPropSpecular2Power)
    SYNC_MAT_PROP(fur, mFur, kMatPropFur)
    SYNC_MAT_PROP(screen_aligned, mScreenAligned, kMatPropScreenAligned)
    SYNC_MAT_PROP(cull, (int &)mCull, kMatPropCull)
    SYNC_MAT_PROP(stencil_mode, (int &)mStencilMode, kMatPropStencilMode)
    SYNC_MAT_PROP(bloom_multiplier, mBloomMultiplier, kMatPropBloomMultiplier)
    SYNC_MAT_PROP(never_fit_to_spline, mNeverFitToSpline, kMatPropNeverFitToSpline)
    SYNC_MAT_PROP(
        allow_distortion_effects, mAllowDistortionEffects, kMatPropAllowDistortionEffects
    )
    SYNC_MAT_PROP(shockwave_mult, mShockwaveMult, kMatPropShockwaveMult)
    SYNC_MAT_PROP(
        world_projection_tiling, mWorldProjectionTiling, kMatPropWorldProjectionTiling
    )
    SYNC_MAT_PROP(
        world_projection_start_blend,
        mWorldProjectionStartBlend,
        kMatPropWorldProjectionStartBlend
    )
    SYNC_MAT_PROP(
        world_projection_end_blend,
        mWorldProjectionEndBlend,
        kMatPropWorldProjectionEndBlend
    )
    SYNC_SUPERCLASS(BaseMaterial)
END_PROPSYNCS

BEGIN_COPYS(MetaMaterial)
    COPY_SUPERCLASS(BaseMaterial)
    CREATE_COPY(MetaMaterial)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMatPropEditActions)
    END_COPYING_MEMBERS
END_COPYS

void MetaMaterial::Init() {
    mMatPropEditActions.resize(kMatPropMax);
    for (int i = 0; i < mMatPropEditActions.size(); i++) {
        if (i != kMatPropMax) {
            SetEditAction((MatProp)i, kNumEditActions);
        }
    }
    for (int i = 0; i < DIM(gDefaultProps); i++) {
        SetEditAction(gDefaultProps[i].prop, gDefaultProps[i].edit_action);
    }
}

BinStream &operator>>(BinStream &bs, MatPropEditAction &a) {
    bs >> (int &)a;
    return bs;
}

BEGIN_LOADS(MetaMaterial)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(BaseMaterial)
    if (gRev > 1) {
        bsrev >> mMatPropEditActions;
    }
END_LOADS

MetaMaterial::MetaMaterial() { Init(); }
