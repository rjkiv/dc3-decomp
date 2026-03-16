#include "rndobj/Shader.h"
#include "Rnd.h"
#include "math/Utl.h"
#include "os/System.h"
#include "rnddx9/RenderState.h"
#include "rndobj/Env.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Env_NG.h"
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
        opts.flags |= 0x80000000000000;
        opts.flags |= (spline->Unk146() << 0x38);
    }
    RndShockwave *selected = RndShockwave::Selected();
    if (selected && !NearlyZero(selected->Amplitude()) && mat->AllowDistortionEffects()
        && !NearlyZero(mat->ShockwaveMult())) {
        opts.flags |= 0x1000000000000000;
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
    bool offscreen = TheNgRnd.Offscreen();
    bool alpha = mat->AlphaWrite();
    if (!mat->ForceAlphaWrite() && opts.flags & 0x400000 || offscreen || alpha) {
        alpha = true;
    }
    TheRenderState.SetColorWriteMask(alpha ? 7 : 8);
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
    if (TheRnd.DrawMode() == 2) {
        shader_type = kShadowmapShader;
    } else if (TheRnd.DrawMode() == Rnd::kDrawVelocity) {
        shader_type = kVelocityObjectShader;
    } else if (TheShaderMgr.Unk18()) {
        shader_type = kDepthVolumeShader;
    }
    if (!b3) {
        if (TheLoadMgr.EditMode() || !UsingCD()) {
            if (!DisplayMatShaderFlagsError(mat, shader_type)) {
                if (mat && TheShaderMgr.ShowMetaMatErrors()) {
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
    int shader20 = TheShaderMgr.Unk20();
    if (TheRnd.DrawMode() == Rnd::kDrawShadowColor || shader20 == 1) {
        TheRenderState.SetCullMode((RndRenderState::CullMode)0);
    } else if (s != kShadowmapShader && shader20 != 3 && TheRnd.DrawMode() != 8) {
        if (shader20 == 2) {
            TheRenderState.SetCullMode((RndRenderState::CullMode)2);
        }
    } else {
        TheRenderState.SetCullMode((RndRenderState::CullMode)6);
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
        || TheRnd.DrawMode() == Rnd::kDrawOcclusion) {
        return true;
    }
    NgEnviron *curEnv = (NgEnviron *)RndEnviron::Current();
    sMatShadersOK = true;
    RndShader *curShader = sShaders[s];
    bool b1824 = mat->UseEnviron() && RndEnviron::Current()->NumLights_Real() != 0;
    if (curShader->CheckError((MatFlagErrorType)0) && !mat->FadeOut()) {
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
    if (curShader->CheckError((MatFlagErrorType)1) && b1824 && !mat->PointLights()
        && curEnv->NumLights_Point()) {
        WarnMatProp("point_lights checked", (NgMat *)mat, curEnv, s);
    }
    if (curShader->CheckError((MatFlagErrorType)2) && !mat->ColorAdjust()
        && curEnv->UseColorAdjust()) {
        WarnMatProp("color_adjust checked", (NgMat *)mat, curEnv, s);
    }
    return sMatShadersOK;
}

bool RndShader::DisplayMatShaderFlagsError(RndMat *mat, ShaderType s) {
    bool ret = false;
    if (TheShaderMgr.ShowShaderErrors()) {
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
    bool select = s == kShadowmapShader || TheRnd.DrawMode() == Rnd::kDrawShadowColor;
    program.Select(select);
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
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    bool isSkinned = TheShaderMgr.Unk10() && (s == kErrorShader || s == kShadowmapShader);
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

void RndShaderParticles::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(false, true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        Cache(s, opts, mat);
    }
}

void RndShaderMultimesh::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
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

void RndShaderStandard::Select(RndMat *mat, ShaderType shader_type, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(
            mat, shader_type, TheShaderMgr.Unk10() != 0, TheShaderMgr.UseAO(), b
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

void RndShaderPostProc::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), false);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        TheRenderState.SetColorWriteMask(0xF);
        Cache(s, opts, mat);
    }
}

void RndShaderDrawRect::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheShaderMgr.DrawRectMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        TheShaderMgr.SetVConstant((VShaderConstant)1, Vector4(1, 1, 1, 1));
        TheShaderMgr.SetPConstant((PShaderConstant)1, Vector4(1, 1, 1, 1));
        CheckForceCull(kStandardShader);
        Cache(kStandardShader, opts, mat);
    }
}

void RndShaderUnwrapUV::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(mat, s, false, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        TheRenderState.SetColorWriteMask(7);
        const Hmx::Color &color = mat->GetColor();
        TheShaderMgr.SetVConstant(
            (VShaderConstant)1, Vector4(color.red, color.green, color.blue, color.alpha)
        );
        TheShaderMgr.SetPConstant(
            (PShaderConstant)1, Vector4(color.red, color.green, color.blue, color.alpha)
        );
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

void RndShaderVelocity::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(mat, s, TheShaderMgr.Unk10() != 0, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(false, false);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

void RndShaderVelocityCamera::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
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

void RndShaderDepthVolume::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(mat, s, TheShaderMgr.Unk10() != 0, false, b)) {
        TheNgStats->mMats++;
        NgMat *ngMat = static_cast<NgMat *>(mat);
        ngMat->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts(ngMat, s, b));
        SetColorWriteMask(opts, mat);
        if (TheShaderMgr.Unk18()) {
            if (TheShaderMgr.Unk24()) {
                TheRenderState.SetBlendOp((RndRenderState::BlendOp)4);
            } else {
                TheRenderState.SetBlendOp((RndRenderState::BlendOp)0);
            }
            TheRenderState.SetBlendEnable(true);
            TheRenderState.SetBlend(
                (RndRenderState::Blend)1,
                (RndRenderState::Blend)1,
                (RndRenderState::Blend)1,
                (RndRenderState::Blend)1
            );
            TheRenderState.SetDepthTestEnable(false);
            TheRenderState.SetDepthWriteEnable(false);
        }
        CheckExtrude();
        TheShaderMgr.SetVConstant((VShaderConstant)1, Vector4(1, 1, 1, 1));
        TheShaderMgr.SetPConstant((PShaderConstant)1, Vector4(1, 1, 1, 1));
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}

void RndShaderFur::Select(RndMat *mat, ShaderType s, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(mat, s, TheShaderMgr.Unk10() != 0, false, b)) {
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

void RndShaderSyncTrack::Select(RndMat *mat, ShaderType shader_type, bool b) {
    if (!mat) {
        mat = TheRnd.DefaultMat();
    }
    TheRenderState.SetFillMode((RndRenderState::FillMode)0);
    if (!RedundantState(
            mat, shader_type, TheShaderMgr.Unk10() != 0, TheShaderMgr.UseAO(), b
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
