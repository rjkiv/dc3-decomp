#include "rndobj/Shader.h"
#include "Rnd.h"
#include "math/Utl.h"
#include "os/System.h"
#include "rndobj/RenderState.h"
#include "rndobj/Env.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Env_NG.h"
#include "rndobj/HiResScreen.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/ShaderProgram.h"
#include "rndobj/Shockwave.h"
#include "rndobj/Spline.h"
#include "rndobj/Stats_NG.h"
#include "utl/Loader.h"
#include "utl/Str.h"
#include <set>

std::set<unsigned int> sWarnings;
RndShaderSimple gShaderSimple;
RndShaderParticles gShaderParticles;
RndShaderMultimesh gShaderMultimesh;
RndShaderStandard gShaderStandard;
RndShaderPostProc gShaderPostProc;
RndShaderDrawRect gShaderDrawRect;
RndShaderUnwrapUV gShaderUnwrapUV;
RndShaderVelocity gShaderVelocity;
RndShaderVelocityCamera gShaderVelocityCamera;
RndShaderDepthVolume gShaderDepthVolume;
RndShaderFur gShaderFur;
RndShaderSyncTrack gShaderSyncTrack;

unsigned int StrHash(const char *str) {
    unsigned int hash = 0;
    int constMult = 0xF8C9;
    for (const unsigned char *p = (const unsigned char *)str; *p != '\0'; p++) {
        hash = hash * constMult + *p;
        constMult *= 0x5C6B7;
    }
    return hash;
}

void CheckDistortionOpts(RndMat *mat, ShaderOptions &opts) {
    RndSpline *spline = RndSpline::GlobalDefaultSpline();
    if (spline && !mat->NeverFitToSpline() && spline->NumCtrlPts() >= 2U) {
        opts.fitToSpline = true;
        opts.splinePulse = spline->HasPulse();
    }
    RndShockwave *selected = RndShockwave::Selected();
    if (selected && !NearlyZero(selected->Amplitude()) && mat->AllowDistortionEffects()
        && !NearlyZero(mat->ShockwaveMult())) {
        opts.shockwave = true;
    }
}

void CheckDistortion(RndMat *mat) {
    RndSpline *spline = RndSpline::GlobalDefaultSpline();
    if (spline && !mat->NeverFitToSpline() && !spline->Manual()
        && spline->NumCtrlPts() >= 2U) {
        spline->PrepareShader();
    }
    RndShockwave *selected = RndShockwave::Selected();
    if (selected && !NearlyZero(selected->Amplitude()) && mat->AllowDistortionEffects()
        && !NearlyZero(mat->ShockwaveMult())) {
        selected->PrepareShader(mat->ShockwaveMult());
    }
}

void SetColorWriteMask(const ShaderOptions &opts, RndMat *mat) {
    bool hdr = opts.hdr;
    bool offscreen = TheNgRnd.Offscreen();
    bool alpha = mat->AlphaWrite();
    alpha = !mat->ForceAlphaWrite() && hdr || offscreen || alpha;
    TheRenderState.SetColorWriteMask(
        alpha ? RndRenderState::kAllChannels : RndRenderState::kColorOnly
    );
}

void CheckShadow();
void CheckExtrude();

void RndShader::Init() {
    sShaders[kBloomShader] = &gShaderSimple;
    sShaders[kDepthVolumeShader] = &gShaderDepthVolume;
    sShaders[kBloomGlareShader] = &gShaderSimple;
    sShaders[kBlurShader] = &gShaderSimple;
    sShaders[kDownsampleDepthShader] = &gShaderSimple;
    sShaders[kDownsample4xShader] = &gShaderSimple;
    sShaders[kDrawRectShader] = &gShaderDrawRect;
    sShaders[kDownsampleShader] = &gShaderSimple;
    sShaders[kMultimeshShader] = &gShaderMultimesh;
    sShaders[kFurShader] = &gShaderFur;
    sShaders[kErrorShader] = &gShaderSimple;
    sShaders[kLineNozShader] = &gShaderSimple;
    sShaders[kMovieShader] = &gShaderSimple;
    sShaders[kMultimeshBBShader] = &gShaderMultimesh;
    sShaders[kLineShader] = &gShaderSimple;
    sShaders[kShadowmapShader] = &gShaderSimple;
    sShaders[kPostprocessErrorShader] = &gShaderSimple;
    sShaders[kPlayerDepthVisShader] = &gShaderSimple;
    sShaders[kParticlesShader] = &gShaderParticles;
    sShaders[kPlayerDepthShellShader] = &gShaderSimple;
    sShaders[kSyncTrackShader] = &gShaderSyncTrack;
    sShaders[kStandardShader] = &gShaderStandard;
    sShaders[kStandardBBShader] = &gShaderStandard;
    sShaders[kPostprocessShader] = &gShaderPostProc;
    sShaders[kPlayerDepthShell2Shader] = &gShaderSimple;
    sShaders[kDepthBuffer3DShader] = &gShaderSimple;
    sShaders[kYUVtoRGBShader] = &gShaderSimple;
    sShaders[kSyncTrackChargeEffectShader] = &gShaderSyncTrack;
    sShaders[kVelocityCameraShader] = &gShaderVelocityCamera;
    sShaders[kUnwrapUVShader] = &gShaderUnwrapUV;
    sShaders[kVelocityObjectShader] = &gShaderVelocity;
    sShaders[kYUVtoBlackAndWhiteShader] = &gShaderSimple;
    sShaders[kPlayerGreenScreenShader] = &gShaderSimple;
    sShaders[kPlayerDepthGreenScreenShader] = &gShaderSimple;
    sShaders[kCrewPhotoShader] = &gShaderSimple;
    sShaders[kTwirlShader] = &gShaderSimple;
    sShaders[kKillAlphaShader] = &gShaderSimple;
    sShaders[kAllWhiteShader] = &gShaderStandard;
}

void RndShader::SelectConfig(RndMat *mat, ShaderType shader_type, bool b3) {
    MILO_ASSERT(shader_type >= ShaderType(0) && shader_type < kMaxShaderTypes, 0x1BB);
    if (TheRnd.DrawMode() == Rnd::kDrawShadowDepth) {
        shader_type = kShadowmapShader;
    } else if (TheRnd.DrawMode() == Rnd::kDrawVelocity) {
        shader_type = kVelocityObjectShader;
    } else if (TheShaderMgr.DepthVolume()) {
        shader_type = kDepthVolumeShader;
    }
    if (!b3) {
        if (TheLoadMgr.EditMode() || !UsingCD()) {
            if (!DisplayMatShaderFlagsError(mat, shader_type)) {
                if (mat && TheShaderMgr.GetShowMetaMatErrors()) {
                    bool metaMat = !mat->GetMetaMaterial();
                    if (metaMat) {
                        shader_type = shader_type == kPostprocessShader
                            ? kPostprocessErrorShader
                            : kErrorShader;
                    }
                }
            }
        }
    }
    RndShader *shader = sShaders[shader_type];
    MILO_ASSERT(shader, 0x1D3);
    shader->Select(mat, shader_type, b3);
}

void RndShader::CheckForceCull(ShaderType s) {
    int shader20 = TheShaderMgr.ForceCullMode();
    if (TheRnd.DrawMode() == Rnd::kDrawExtrude || shader20 == 1) {
        TheRenderState.SetCullMode(RndRenderState::kCullModeNone);
    } else if (s != kShadowmapShader && shader20 != 3
               && TheRnd.DrawMode() != Rnd::kDrawReversed) {
        if (shader20 == 2) {
            TheRenderState.SetCullMode(RndRenderState::kCullModeCW);
        }
    } else {
        TheRenderState.SetCullMode(RndRenderState::kCullModeCCW);
    }
}

bool RndShader::RedundantState(
    const RndMat *mat, ShaderType s, bool skinned, bool useAO, bool b5
) {
    if (!b5 && mat && (NgMat *)mat == NgMat::Current() && !mat->Dirty()
        && s == sCurrentShader && skinned == sCurrentSkinned && useAO == sCurrentUseAO) {
        if (s == kStandardShader || s == kStandardBBShader || s == kParticlesShader
            || s == kMultimeshShader || s == kMultimeshBBShader || s == kSyncTrackShader
            || s == kSyncTrackChargeEffectShader || s == kAllWhiteShader) {
            return true;
        }
    }
    sCurrentUseAO = useAO;
    sCurrentShader = s;
    sCurrentSkinned = skinned;
    return false;
}

void RndShader::ShaderWarn(const char *msg) {
    unsigned int hash = StrHash(msg);
    if (sWarnings.end() == sWarnings.find(hash)) {
        MILO_NOTIFY(msg);
        sWarnings.insert(hash);
    }
    if (TheLoadMgr.EditMode()) {
        Debug::ModalType ty = Debug::kModalNotify;
        if (mModalCallback) {
            StackString<1024> str(msg);
            (*mModalCallback)(ty, str, true);
        }
    }
}

void RndShader::WarnMatProp(const char *prop, NgMat *mat, NgEnviron *env, ShaderType s) {
    ShaderWarn(MakeString(
        "[%s] must have %s.  (%s, %s)",
        PathName(mat),
        prop,
        PathName(env),
        ShaderTypeName(s)
    ));
    sMatShadersOK = false;
}

bool RndShader::MatShaderFlagsOK(RndMat *mat, ShaderType s) {
    if (!mat || TheRnd.DefaultEnv() == RndEnviron::Current()
        || TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        return true;
    }
    NgEnviron *curEnv = (NgEnviron *)RndEnviron::Current();
    sMatShadersOK = true;
    RndShader *curShader = sShaders[s];
    bool b1824 = mat->UseEnviron() && RndEnviron::Current()->NumLights_Real() != 0;
    if (curShader->CheckError(kFadeOutErr) && !mat->FadeOut()) {
        bool fadeoutCheck = curEnv->FadeOut() && curEnv->FadeEnd() != curEnv->FadeStart();
        if (fadeoutCheck) {
            WarnMatProp("fadeout checked", (NgMat *)mat, curEnv, s);
        }
    } else if (mat->FadeOut()) {
        bool fadeoutUncheck =
            curEnv->FadeOut() && curEnv->FadeEnd() != curEnv->FadeStart();
        if (!fadeoutUncheck) {
            WarnMatProp("fadeout unchecked", (NgMat *)mat, curEnv, s);
        }
    }
    if (curShader->CheckError(kLightingErr) && b1824 && !mat->PointLights()
        && curEnv->NumLights_Point()) {
        WarnMatProp("point_lights checked", (NgMat *)mat, curEnv, s);
    }
    if (curShader->CheckError(kColorXfmErr) && !mat->ColorAdjust()
        && curEnv->UseColorXfm()) {
        WarnMatProp("color_adjust checked", (NgMat *)mat, curEnv, s);
    }
    return sMatShadersOK;
}

bool RndShader::DisplayMatShaderFlagsError(RndMat *mat, ShaderType s) {
    bool ret = false;
    if (TheShaderMgr.GetShowShaderErrors()) {
        ret = !MatShaderFlagsOK(mat, s);
    }
    return ret;
}

void RndShader::Cache(ShaderType s, ShaderOptions opts, RndMat *mat) {
    RndShaderProgram &program = TheShaderMgr.FindShader(s, opts);
    if (!program.Cached()) {
        if (!program.Cache(s, opts, nullptr, nullptr)
            && (UsingCD() || !TheShaderMgr.CacheShaders())) {
            MatShaderFlagsOK(mat, s);
        }
    }
    bool select = s == kShadowmapShader || TheRnd.DrawMode() == Rnd::kDrawExtrude;
    program.Select(select);
}

// which TEX_GEN variation of the shader handles this tex gen mode
static int TexGenOption(TexGen texGen) {
    switch (texGen) {
    case kTexGenSphere:
        return 1;
    case kTexGenProjected:
        return 2;
    case kTexGenEnviron:
        return 3;
    default:
        return 0;
    }
}

u64 RndShaderSimple::CalcShaderOpts(NgMat *mat, ShaderType stype, bool) {
    ShaderOptions opts;
    switch (stype) {
    case kBlurShader:
        opts.numTaps = TheShaderMgr.NumTaps() - 1;
        break;
    case kErrorShader:
        opts.skinned = TheShaderMgr.NumBones() ? true : false;
        opts.displayError = TheShaderMgr.GetShaderErrorDisplay();
        break;
    case kPostprocessErrorShader:
        opts.displayError = TheShaderMgr.GetShaderErrorDisplay();
        break;
    case kShadowmapShader:
        opts.skinned = TheShaderMgr.NumBones() ? true : false;
        break;
    case kMovieShader:
        opts.movieGrayscale = mat->SpecularMap() == nullptr;
        opts.movieAlpha = mat->NormalMap() ? true : false;
        break;
    default:
        break;
    }
    opts.hiResScreen = TheHiResScreen.IsActive();
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        opts.value = 0;
    }
    return opts.value;
}

void RndShaderSimple::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        if (s == kLineNozShader) {
            mat = TheShaderMgr.DrawHighlightMat();
            mat->SetZMode(kZModeForce);
            s = kLineShader;
        } else {
            mat = TheRnd.DefaultMat();
        }
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    bool isSkinned =
        TheShaderMgr.NumBones() && (s == kErrorShader || s == kShadowmapShader);
    if (!RedundantState(mat, s, isSkinned, TheShaderMgr.UseAO(), b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

u64 RndShaderParticles::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    ShaderOptions opts;
    opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
    opts.prelit = true;
    opts.texGen = TexGenOption(mat->GetTexGen());
    opts.hasLightsReal = false;
    opts.hasLightsApprox = false;
    RndEnviron *env = RndEnviron::Current();
    bool fade;
    if (precache) {
        fade = mat->FadeOut();
    } else {
        fade = env->FadeOut() && env->FadeEnd() != env->FadeStart();
    }
    opts.hdr = !fade
        && !(precache ? TheShaderMgr.GetPrecacheOffscreen() : TheNgRnd.Offscreen())
        && mat->AllowHDR();
    opts.colorXfm = precache ? mat->ColorXfm() : env->UseColorXfm();
    opts.intensify = mat->Intensify();
    if (fade) {
        const Hmx::Color &fadeColor = mat->FadeColor();
        TheShaderMgr.SetPConstant(
            kPShader_FadeColor,
            Vector4(fadeColor.red, fadeColor.green, fadeColor.blue, fadeColor.alpha)
        );
        opts.fade = mat->GetFadeType();
    }
    if (mat->GetRefractEnabled(precache) && mat->GetRefractNormalMap()) {
        opts.refractWorld = true;
    }
    opts.fog = mat->AllowFog() && mat->Fog();
    if (TheRnd.DrawMode() == Rnd::kDrawSoftDepth) {
        opts.softDepthBlend = true;
    }
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        opts.value = 0;
    }
    opts.showShaderCost = TheRnd.ShowShaderCost();
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderParticles::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(false, true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        Cache(s, opts, mat);
    }
}

u64 RndShaderMultimesh::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        return 0;
    }
    NgEnviron *env = (NgEnviron *)RndEnviron::Current();
    ShaderOptions opts;
    opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
    opts.prelit = mat->PreLit();
    opts.hasLightsReal = mat->UseEnviron() && env->NumLights_Real() > 0;
    opts.hasLightsApprox = mat->UseEnviron() && env->NumLights_Approx() > 0;
    if (opts.hasLightsReal || opts.hasLightsApprox) {
        opts.specular = mat->Specular().Pack() != 0;
        if (TheShaderMgr.AllowPerPixel() && mat->PerPixelLighting()) {
            opts.perPixel = true;
            opts.normalMap = mat->NormalMap() ? true : false;
            opts.normDetail = mat->NormDetail();
            opts.normalFlip = mat->GetCull() == kCullBackwards ? true : false;
            opts.specularMap = opts.specular && mat->SpecularMap();
            opts.rimlightEnabled = mat->GetRimColor().Pack() != 0;
            opts.rimlightUnder = opts.rimlightEnabled && mat->GetRimLightUnder();
            opts.rimlightMap = opts.rimlightEnabled && mat->GetRimMap();
        }
        if (mat->EnvironMap()) {
            opts.environMapFalloff = mat->EnvironMapFalloff();
            opts.environMap = true;
            opts.environMapSpecMask = opts.specularMap && mat->EnvironMapSpecMask();
        }
        opts.numLightPoint = env->NumLights_Point();
    }
    opts.glowMap = mat->EmissiveMap() ? true : false;
    opts.intensify = mat->Intensify();
    opts.texGen = TexGenOption(mat->GetTexGen());
    bool fade;
    if (precache) {
        fade = mat->FadeOut();
    } else {
        fade = env->FadeOut() && env->FadeEnd() != env->FadeStart();
    }
    opts.hdr = !fade
        && !(precache ? TheShaderMgr.GetPrecacheOffscreen() : TheNgRnd.Offscreen())
        && mat->AllowHDR();
    opts.fog = mat->AllowFog() && mat->Fog();
    opts.billboard = shader_type == kMultimeshBBShader;
    opts.colorXfm = precache ? mat->ColorXfm() : env->UseColorXfm();
    opts.customVariation = mat->GetShaderVariation();
    opts.colorMod = mat->GetColorModFlags();
    if (!opts.prelit && TheShaderMgr.UseAO() && env->GetAmbientOcclusionEnabled()
        && env->GetAmbientOcclusionStrength() > 0.003f) {
        opts.ambientOcclusionOn = true;
    }
    opts.toneMapping = env->GetUseToneMapping();
    if (fade) {
        const Hmx::Color &fadeColor = mat->FadeColor();
        TheShaderMgr.SetPConstant(
            kPShader_FadeColor,
            Vector4(fadeColor.red, fadeColor.green, fadeColor.blue, fadeColor.alpha)
        );
        opts.fade = mat->GetFadeType();
    }
    CheckDistortionOpts(mat, opts);
    bool projLights = mat->PerfSettings().mRecvProjLights && env->NumLights_Proj() > 0;
    opts.numLightProj = projLights ? env->NumLights_Proj() : 0;
    opts.showShaderCost = TheRnd.ShowShaderCost();
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderMultimesh::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, false, TheShaderMgr.UseAO(), b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        CheckForceCull(kMultimeshShader);
        CheckDistortion(mat);
        Cache(kMultimeshShader, opts, mat);
    }
}

u64 RndShaderStandard::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    NgEnviron *env = (NgEnviron *)RndEnviron::Current();
    ShaderOptions opts;
    opts.skinned = TheShaderMgr.NumBones() ? true : false;
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        return opts.value;
    }
    if (TheRnd.DrawMode() == Rnd::kDrawFastAndCheap) {
        opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
        opts.prelit = mat->PreLit();
        if (mat->UseEnviron()) {
            opts.fastAndCheapLighting = true;
        }
        return opts.value;
    }
    bool fade;
    if (precache) {
        fade = mat->FadeOut();
    } else {
        fade = env->FadeOut() && env->FadeEnd() != env->FadeStart();
    }
    opts.hdr = mat->AllowHDR() && !fade
        && !(precache ? TheShaderMgr.GetPrecacheOffscreen() : TheNgRnd.Offscreen());
    opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
    opts.prelit = mat->PreLit();
    opts.hasLightsReal = mat->UseEnviron() && env->NumLights_Real() > 0;
    opts.hasLightsApprox = mat->UseEnviron() && env->NumLights_Approx() > 0;
    if (opts.hasLightsReal || opts.hasLightsApprox) {
        opts.specular = mat->Specular().Pack() != 0;
        if (TheShaderMgr.AllowPerPixel() && mat->PerPixelLighting()) {
            opts.perPixel = true;
            opts.normalMap = mat->NormalMap() ? true : false;
            opts.normDetail = mat->NormDetail();
            opts.normalFlip = mat->GetCull() == kCullBackwards ? true : false;
            opts.specularMap = opts.specular && mat->SpecularMap();
            opts.rimlightEnabled = mat->GetRimColor().Pack() != 0;
            opts.rimlightUnder = opts.rimlightEnabled && mat->GetRimLightUnder();
            opts.rimlightMap = opts.rimlightEnabled && mat->GetRimMap();
            opts.shadowMap = TheRnd.GetShadowCam() ? true : false;
        }
        if (mat->EnvironMap()) {
            opts.environMapFalloff = mat->EnvironMapFalloff();
            opts.environMap = true;
            opts.environMapSpecMask = opts.specularMap && mat->EnvironMapSpecMask();
        }
        bool projLights =
            mat->PerfSettings().mRecvProjLights && env->NumLights_Proj() > 0;
        bool pointCubeTex = mat->PerfSettings().mRecvPointCubeTex
            && env->NumLights_Point() > 0 && env->HasPointCubeTex();
        opts.anisotropic = mat->Anisotropy() > 0;
        opts.numLightPoint = env->NumLights_Point();
        opts.numLightProj = projLights ? env->NumLights_Proj() : 0;
        opts.projLightMultiply =
            projLights && env->GetProjectedBlend() == RndLight::kMultiply;
        opts.enablePointCubeTex = pointCubeTex;
    }
    if (mat->GetRefractEnabled(precache) && mat->GetRefractNormalMap()) {
        opts.refractWorld = true;
    }
    opts.glowMap = mat->EmissiveMap() ? true : false;
    opts.screenAligned = mat->ScreenAligned();
    opts.intensify = mat->Intensify();
    opts.texGen = TexGenOption(mat->GetTexGen());
    opts.fog = mat->AllowFog() && mat->Fog();
    opts.billboard = shader_type == kStandardBBShader;
    opts.colorXfm = precache ? mat->ColorXfm() : env->UseColorXfm();
    opts.customVariation = mat->GetShaderVariation();
    opts.colorMod = mat->GetColorModFlags();
    if (!opts.prelit && TheShaderMgr.UseAO() && env->GetAmbientOcclusionEnabled()
        && env->GetAmbientOcclusionStrength() > 0.003f) {
        opts.ambientOcclusionOn = true;
    }
    opts.toneMapping = env->GetUseToneMapping();
    if (fade && !opts.fog) {
        const Hmx::Color &fadeColor = mat->FadeColor();
        TheShaderMgr.SetPConstant(
            kPShader_FadeColor,
            Vector4(fadeColor.red, fadeColor.green, fadeColor.blue, fadeColor.alpha)
        );
        opts.fade = mat->GetFadeType();
    }
    CheckDistortionOpts(mat, opts);
    opts.showShaderCost = TheRnd.ShowShaderCost();
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderStandard::Select(RndMat *mat, ShaderType shader_type, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(
            mat, shader_type, TheShaderMgr.NumBones() != 0, TheShaderMgr.UseAO(), b
        )) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        CheckShadow();
        ShaderOptions opts(CalcShaderOpts(ngMat, shader_type, b));
        MILO_ASSERT((shader_type == kStandardShader) || (shader_type == kStandardBBShader) || (shader_type == kAllWhiteShader), 0x4BB);
        if (shader_type == kStandardBBShader) {
            shader_type = kStandardShader;
        }
        SetColorWriteMask(opts, mat);
        CheckExtrude();
        CheckForceCull(shader_type);
        CheckDistortion(mat);
        Cache(shader_type, opts, mat);
    }
}

u64 RndShaderPostProc::CalcShaderOpts(NgMat *, ShaderType, bool) {
    bool noiseMidtone = TheShaderMgr.PostNoiseMidtone();
    bool spotlight = TheShaderMgr.PostSpotlight();
    bool velocity = TheShaderMgr.PostVelocity();
    bool chromaticSharpen = TheShaderMgr.PostChromaticSharpen();
    bool softDepthBlend = TheShaderMgr.PostSoftDepth();
    bool hueConverge = TheShaderMgr.PostHueConverge();
    bool colorXfm = TheShaderMgr.PostColorXfm();
    bool noise = TheShaderMgr.PostNoise();
    bool dof = TheShaderMgr.PostDOF();
    bool bloom = TheShaderMgr.PostBloom();
    bool glare = TheShaderMgr.PostGlare();
    bool blendPrevious = TheShaderMgr.PostBlendPrevious();
    bool copyPrevious = TheShaderMgr.PostCopyPrevious();
    bool kaleidoscope = TheShaderMgr.PostKaleidoscope();
    bool resamp = TheShaderMgr.PostResamp();
    bool posterize = TheShaderMgr.PostPosterize();
    int hallOfTime = TheShaderMgr.PostHallOfTime();
    bool motionBlur = TheShaderMgr.PostMotionBlur();
    bool gradientMap = TheShaderMgr.PostGradientMap();
    bool refract = TheShaderMgr.PostRefract();
    bool chromaticAberration = TheShaderMgr.PostChromaticAberration();
    bool vignette = TheShaderMgr.PostVignette();
    TheShaderMgr.SetPostColorXfm(false);
    TheShaderMgr.SetPostNoise(false);
    TheShaderMgr.SetPostNoiseMidtone(false);
    TheShaderMgr.SetPostDOF(false);
    TheShaderMgr.SetPostBloom(false);
    TheShaderMgr.SetPostGlare(false);
    TheShaderMgr.SetPostBlendPrevious(false);
    TheShaderMgr.SetPostCopyPrevious(false);
    TheShaderMgr.SetPostKaleidoscope(false);
    TheShaderMgr.SetPostResamp(false);
    TheShaderMgr.SetPostSpotlight(false);
    TheShaderMgr.SetPostPosterize(false);
    TheShaderMgr.SetPostMotionBlur(false);
    TheShaderMgr.SetPostVelocity(false);
    TheShaderMgr.SetPostGradientMap(false);
    TheShaderMgr.SetPostHueConverge(false);
    TheShaderMgr.SetPostRefract(false);
    TheShaderMgr.SetPostChromaticAberration(false);
    TheShaderMgr.SetPostChromaticSharpen(false);
    TheShaderMgr.SetPostHallOfTime(0);
    TheShaderMgr.SetPostVignette(false);
    TheShaderMgr.SetPostSoftDepth(false);
    ShaderOptions opts;
    opts.colorXfm = colorXfm;
    opts.noise = noise;
    opts.noiseMidtone = noiseMidtone;
    opts.dof = dof;
    opts.bloom = bloom;
    opts.glare = glare;
    opts.blendPrevious = blendPrevious;
    opts.copyPrevious = copyPrevious;
    opts.kaleidoscope = kaleidoscope;
    opts.resamp = resamp;
    opts.spotlight = spotlight;
    opts.posterize = posterize;
    opts.motionBlur = motionBlur;
    opts.velocity = velocity;
    opts.gradientMap = gradientMap;
    opts.hueConverge = hueConverge;
    opts.refract = refract;
    opts.chromaticAberration = chromaticAberration;
    opts.chromaticSharpen = chromaticSharpen;
    opts.hallOfTime = hallOfTime;
    opts.vignette = vignette;
    opts.softDepthBlend = softDepthBlend;
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderPostProc::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), false);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        TheRenderState.SetColorWriteMask(0xF);
        Cache(s, opts, mat);
    }
}

u64 RndShaderDrawRect::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        return 0;
    }
    ShaderOptions opts;
    opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
    opts.prelit = mat->PreLit();
    bool offscreen =
        precache ? TheShaderMgr.GetPrecacheOffscreen() : TheNgRnd.Offscreen();
    opts.hdr = !offscreen && mat->AllowHDR();
    opts.showShaderCost = TheRnd.ShowShaderCost();
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderDrawRect::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheShaderMgr.DrawRectMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        TheShaderMgr.SetVConstant(kVShader_AmbientCol, Vector4(1, 1, 1, 1));
        TheShaderMgr.SetPConstant(kPShader_AmbientCol, Vector4(1, 1, 1, 1));
        CheckForceCull(kStandardShader);
        Cache(kStandardShader, opts, mat);
    }
}

u64 RndShaderUnwrapUV::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    ShaderOptions opts;
    opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
    opts.prelit = true;
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderUnwrapUV::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        TheRenderState.SetColorWriteMask(7);
        const Hmx::Color &color = mat->GetColor();
        TheShaderMgr.SetVConstant(
            kVShader_AmbientCol, Vector4(color.red, color.green, color.blue, color.alpha)
        );
        TheShaderMgr.SetPConstant(
            kPShader_AmbientCol, Vector4(color.red, color.green, color.blue, color.alpha)
        );
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

u64 RndShaderVelocity::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    ShaderOptions opts;
    opts.skinned = TheShaderMgr.NumBones() > 0;
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderVelocity::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, TheShaderMgr.NumBones() != 0, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(false, false);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

u64 RndShaderVelocityCamera::CalcShaderOpts(
    NgMat *mat, ShaderType shader_type, bool precache
) {
    ShaderOptions opts;
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderVelocityCamera::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(false, false);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

u64 RndShaderDepthVolume::CalcShaderOpts(
    NgMat *mat, ShaderType shader_type, bool precache
) {
    ShaderOptions opts;
    opts.shape = TheShaderMgr.DepthShape();
    opts.skinned = TheShaderMgr.NumBones() ? true : false;
    opts.extrude = TheRnd.DrawMode() == Rnd::kDrawExtrude;
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderDepthVolume::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, TheShaderMgr.NumBones() != 0, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        if (TheShaderMgr.DepthVolume()) {
            if (TheShaderMgr.Subtract()) {
                TheRenderState.SetBlendOp(RndRenderState::kBlendOpRevSubtract);
            } else {
                TheRenderState.SetBlendOp(RndRenderState::kBlendOpAdd);
            }
            TheRenderState.SetBlendEnable(true);
            TheRenderState.SetBlend(
                RndRenderState::kBlendOne,
                RndRenderState::kBlendOne,
                RndRenderState::kBlendOne,
                RndRenderState::kBlendOne
            );
            TheRenderState.SetDepthTestEnable(false);
            TheRenderState.SetDepthWriteEnable(false);
        }
        CheckExtrude();
        TheShaderMgr.SetVConstant(kVShader_AmbientCol, Vector4(1, 1, 1, 1));
        TheShaderMgr.SetPConstant(kPShader_AmbientCol, Vector4(1, 1, 1, 1));
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

u64 RndShaderFur::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    NgEnviron *env = (NgEnviron *)RndEnviron::Current();
    ShaderOptions opts;
    opts.skinned = TheShaderMgr.NumBones() ? true : false;
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        return opts.value;
    }
    opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
    opts.prelit = mat->PreLit();
    opts.hasLightsReal = mat->UseEnviron() && env->NumLights_Real() > 0;
    opts.hasLightsApprox = mat->UseEnviron() && env->NumLights_Approx() > 0;
    if (opts.hasLightsReal || opts.hasLightsApprox) {
        if (TheShaderMgr.AllowPerPixel() && mat->PerPixelLighting()) {
            opts.perPixel = false;
            opts.shadowMap = TheRnd.GetShadowCam() ? true : false;
        }
        bool projLights =
            mat->PerfSettings().mRecvProjLights && env->NumLights_Proj() > 0;
        bool pointCubeTex = mat->PerfSettings().mRecvPointCubeTex
            && env->NumLights_Point() > 0 && env->HasPointCubeTex();
        opts.anisotropic = mat->Anisotropy() > 0;
        opts.numLightPoint = env->NumLights_Point();
        opts.numLightProj = projLights ? env->NumLights_Proj() : 0;
        opts.projLightMultiply =
            projLights && env->GetProjectedBlend() == RndLight::kMultiply;
        opts.enablePointCubeTex = pointCubeTex;
    }
    opts.screenAligned = mat->ScreenAligned();
    opts.fog = (precache ? mat->Fog() : env->FogEnable()) && env->FogEnable();
    opts.colorXfm = precache ? mat->ColorXfm() : env->UseColorXfm();
    opts.furDetail = mat->Fur() && mat->Fur()->mFurDetail ? true : false;
    bool fade;
    if (precache) {
        fade = mat->FadeOut();
    } else {
        fade = env->FadeOut() && env->FadeEnd() != env->FadeStart();
    }
    if (fade && !opts.fog) {
        const Hmx::Color &fadeColor = mat->FadeColor();
        TheShaderMgr.SetPConstant(
            kPShader_FadeColor,
            Vector4(fadeColor.red, fadeColor.green, fadeColor.blue, fadeColor.alpha)
        );
        opts.fade = mat->GetFadeType();
    }
    opts.showShaderCost = TheRnd.ShowShaderCost();
    opts.hiResScreen = TheHiResScreen.IsActive();
    return opts.value;
}

void RndShaderFur::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(mat, s, TheShaderMgr.NumBones() != 0, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(false, true);
        CheckShadow();
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

u64 RndShaderSyncTrack::CalcShaderOpts(NgMat *mat, ShaderType shader_type, bool precache) {
    NgEnviron *env = (NgEnviron *)RndEnviron::Current();
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor) {
        return 0;
    }
    ShaderOptions opts;
    bool fade;
    if (precache) {
        fade = mat->FadeOut();
    } else {
        fade = env->FadeOut() && env->FadeEnd() != env->FadeStart();
    }
    opts.hdr = mat->AllowHDR() && !fade
        && !(precache ? TheShaderMgr.GetPrecacheOffscreen() : TheNgRnd.Offscreen());
    opts.diffuseMap = mat->GetDiffuseTex() ? true : false;
    opts.prelit = mat->PreLit();
    opts.hasLightsReal = mat->UseEnviron() && env->NumLights_Real() > 0;
    opts.hasLightsApprox = mat->UseEnviron() && env->NumLights_Approx() > 0;
    if (opts.hasLightsReal || opts.hasLightsApprox) {
        opts.specular = mat->Specular().Pack() != 0;
        if (TheShaderMgr.AllowPerPixel() && mat->PerPixelLighting()) {
            opts.perPixel = true;
            opts.normalMap = mat->NormalMap() ? true : false;
            opts.normDetail = mat->NormDetail();
            opts.normalFlip = mat->GetCull() == kCullBackwards ? true : false;
            opts.specularMap = opts.specular && mat->SpecularMap();
            opts.rimlightEnabled = mat->GetRimColor().Pack() != 0;
            opts.rimlightUnder = opts.rimlightEnabled && mat->GetRimLightUnder();
            opts.rimlightMap = opts.rimlightEnabled && mat->GetRimMap();
            opts.shadowMap = TheRnd.GetShadowCam() ? true : false;
        }
        if (mat->EnvironMap()) {
            opts.environMapFalloff = mat->EnvironMapFalloff();
            opts.environMap = true;
            opts.environMapSpecMask = opts.specularMap && mat->EnvironMapSpecMask();
        }
        bool projLights =
            mat->PerfSettings().mRecvProjLights && env->NumLights_Proj() > 0;
        bool pointCubeTex = mat->PerfSettings().mRecvPointCubeTex
            && env->NumLights_Point() > 0 && env->HasPointCubeTex();
        opts.anisotropic = mat->Anisotropy() > 0;
        opts.numLightPoint = env->NumLights_Point();
        opts.numLightProj = projLights ? env->NumLights_Proj() : 0;
        opts.projLightMultiply =
            projLights && env->GetProjectedBlend() == RndLight::kMultiply;
        opts.enablePointCubeTex = pointCubeTex;
    }
    if (mat->GetRefractEnabled(precache) && mat->GetRefractNormalMap()) {
        opts.refractWorld = true;
    }
    opts.glowMap = mat->EmissiveMap() ? true : false;
    opts.screenAligned = mat->ScreenAligned();
    opts.intensify = mat->Intensify();
    opts.texGen = TexGenOption(mat->GetTexGen());
    opts.fog = mat->AllowFog() && mat->Fog();
    opts.billboard = false;
    opts.colorXfm = precache ? mat->ColorXfm() : env->UseColorXfm();
    opts.customVariation = mat->GetShaderVariation();
    opts.colorMod = mat->GetColorModFlags();
    if (!opts.prelit && TheShaderMgr.UseAO() && env->GetAmbientOcclusionEnabled()
        && env->GetAmbientOcclusionStrength() > 0.003f) {
        opts.ambientOcclusionOn = true;
    }
    opts.toneMapping = env->GetUseToneMapping();
    if (fade && !opts.fog) {
        const Hmx::Color &fadeColor = mat->FadeColor();
        TheShaderMgr.SetPConstant(
            kPShader_FadeColor,
            Vector4(fadeColor.red, fadeColor.green, fadeColor.blue, fadeColor.alpha)
        );
        opts.fade = mat->GetFadeType();
    }
    opts.showShaderCost = TheRnd.ShowShaderCost();
    opts.hiResScreen = TheHiResScreen.IsActive();
    opts.fitToSpline = true;
    RndSpline *spline = RndSpline::GlobalDefaultSpline();
    if (spline) {
        opts.splinePulse = spline->HasPulse();
    }
    opts.syncTrackChargeEffect = shader_type == kSyncTrackChargeEffectShader;
    return opts.value;
}

void RndShaderSyncTrack::Select(RndMat *mat, ShaderType shader_type, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode(RndRenderState::kFillModeSolid);
    if (!RedundantState(
            mat, shader_type, TheShaderMgr.NumBones() != 0, TheShaderMgr.UseAO(), b
        )) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        CheckShadow();
        ShaderOptions opts(CalcShaderOpts(ngMat, shader_type, b));
        MILO_ASSERT((shader_type == kSyncTrackShader) || (shader_type == kSyncTrackChargeEffectShader), 0x749);
        if (shader_type == kSyncTrackChargeEffectShader) {
            shader_type = kSyncTrackShader;
        }
        SetColorWriteMask(opts, mat);
        CheckExtrude();
        CheckForceCull(shader_type);
        Cache(shader_type, opts, mat);
    }
}
