#include "rndobj/Shader.h"
#include "Rnd.h"
#include "os/System.h"
#include "rnddx9/RenderState.h"
#include "rndobj/Env.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Env_NG.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/ShaderProgram.h"
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

void CheckDistortionOpts(RndMat *, const ShaderOptions &);
void CheckDistortion(RndMat *);
void SetColorWriteMask(const ShaderOptions &, RndMat *);
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

void RndShader::CheckForceCull(ShaderType s) {
    int shader20 = TheShaderMgr.Unk20();
    if (TheRnd.GetDrawMode() == Rnd::kDrawShadowColor || shader20 == 1) {
        TheRenderState.SetCullMode((RndRenderState::CullMode)0);
    } else if (s != kShadowmapShader && shader20 != 3 && TheRnd.GetDrawMode() != 8) {
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
        || TheRnd.GetDrawMode() == Rnd::kDrawOcclusion) {
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

void RndShader::SelectConfig(RndMat *mat, ShaderType shader_type, bool b3) {
    MILO_ASSERT(shader_type >= ShaderType(0) && shader_type < kMaxShaderTypes, 0x1BB);
    if (TheRnd.GetDrawMode() == 2) {
        shader_type = kShadowmapShader;
    } else if (TheRnd.GetDrawMode() == 6) {
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

void RndShader::Cache(ShaderType s, ShaderOptions opts, RndMat *mat) {
    RndShaderProgram &program = TheShaderMgr.FindShader(s, opts);
    if (!program.Cached()) {
        if (!program.Cache(s, opts, nullptr, nullptr)
            && (UsingCD() || !TheShaderMgr.CacheShaders())) {
            MatShaderFlagsOK(mat, s);
        }
    }
    bool select = s == kShadowmapShader || TheRnd.GetDrawMode() == Rnd::kDrawShadowColor;
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
        ((NgMat *)mat)->SetupShader(TheShaderMgr.AllowPerPixel(), true);
        ShaderOptions opts(CalcShaderOpts((NgMat *)mat, s, b));
        SetColorWriteMask(opts, mat);
        CheckForceCull(s);
        Cache(s, opts, mat);
    }
}
