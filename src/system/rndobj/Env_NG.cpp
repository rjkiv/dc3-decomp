#include "rndobj/Env_NG.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "os/Debug.h"
#include "rndobj/BoxMap.h"
#include "rndobj/Env.h"
#include "rndobj/Lit.h"
#include "rndobj/Lit_NG.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/RenderState.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Stats_NG.h"

namespace {
    Hmx::Matrix4 sIdentityXfm(
        Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1)
    );

    void ClearLightTransforms() {
        TheShaderMgr.SetVConstant4x3((VShaderConstant)0xDD, sIdentityXfm);
        TheShaderMgr.SetPConstant4x3((PShaderConstant)0xDD, sIdentityXfm);
    }

    void ClearPointCubeTex() {
        TheShaderMgr.SetPConstant((PShaderConstant)0xD, TheRnd.GetCubeTexWhite());
        TheRenderState.SetTextureClamp(0xD, RndRenderState::kClampModeMirror);
        TheRenderState.SetTextureFilter(0xD, RndRenderState::kFilterModeLinear, false);
    }

    void ClearLightRegisters(int i1) {
        static Vector4 vb0(0, 0, 0, 1);
        static Vector4 va0(0, 0, 0, 0);
        TheShaderMgr.SetVConstant((VShaderConstant)(0x3E + i1), va0);
        TheShaderMgr.SetPConstant((PShaderConstant)(0x3E + i1), va0);
        TheShaderMgr.SetVConstant(
            (VShaderConstant)(0x42 + i1), Vector4(vb0.x, vb0.y, vb0.z, vb0.w)
        );
        TheShaderMgr.SetPConstant(
            (PShaderConstant)(0x42 + i1), Vector4(vb0.x, vb0.y, vb0.z, vb0.w)
        );
    }

    bool CheckPointLight(NgLight &light) {
        if (!light.Showing()) {
            return false;
        } else {
            Hmx::Color color = light.GetColor();
            return color.Pack();
        }
    }

    bool CheckProjLight(NgLight &light) {
        if (!light.Showing()) {
            return false;
        }
        if (light.GetProjectedBlend() == 0 && !light.GetTexture()) {
            return false;
        }
        if (light.GetProjectedBlend() == 1) {
            bool hasShadowOverrides =
                light.GetShadowOverride() && !light.GetShadowOverride()->empty();
            if (!hasShadowOverrides && light.GetShadowObjects().empty()) {
                return false;
            }
        }
        Hmx::Color color = light.GetColor();
        if (color.Pack()) {
            light.CheckShadowMap();
            return true;
        } else {
            return false;
        }
    }

    bool SetPointLightRegisters(int i1, RndLight &light, bool &b3) {
        b3 = false;
        if (!light.Showing()) {
            return false;
        }
        Hmx::Color color = light.GetColor();
        if (!color.Pack()) {
            return false;
        }
        const Transform &worldXfm = light.WorldXfm();
        Vector4 ve0;
        ve0.x = worldXfm.v.x;
        ve0.y = worldXfm.v.y;
        ve0.z = worldXfm.v.z;
        float f9;
        if (light.FalloffStart() < light.Range()) {
            ve0.w = 1 / (light.FalloffStart() - light.Range());
            f9 = -(light.Range() * ve0.w);
        } else {
            f9 = 1;
            ve0.w = 0;
        }
        TheShaderMgr.SetVConstant((VShaderConstant)(0x3E + i1), ve0);
        TheShaderMgr.SetPConstant((PShaderConstant)(0x3E + i1), ve0);
        TheShaderMgr.SetVConstant(
            (VShaderConstant)(0x42 + i1), Vector4(color.red, color.green, color.blue, f9)
        );
        TheShaderMgr.SetPConstant(
            (PShaderConstant)(0x42 + i1), Vector4(color.red, color.green, color.blue, f9)
        );
        if (light.GetCubeTexture() && i1 == 0) {
            TheShaderMgr.SetPConstant((PShaderConstant)0xD, light.GetCubeTexture());
            TheRenderState.SetTextureClamp(0xD, RndRenderState::kClampModeMirror);
            TheRenderState.SetTextureFilter(0xD, RndRenderState::kFilterModeLinear, false);
            Transform tfd0 = light.WorldXfm();
            TheShaderMgr.SetVConstant4x3((VShaderConstant)0xDD, Hmx::Matrix4(tfd0));
            TheShaderMgr.SetPConstant4x3((PShaderConstant)0xDD, Hmx::Matrix4(tfd0));
            b3 = true;
        }
        return true;
    }

    bool SetProjLightRegisters(int i1, int i2, NgLight &light) {
        if (!light.Showing()) {
            return false;
        }
        if (light.GetTexture() || light.GetProjectedBlend()) {
            Hmx::Color color = light.GetColor();
            if (!color.Pack()) {
                return false;
            }
            const Transform &world = light.WorldXfm();
            float x = -world.m.y.x;
            float y = -world.m.y.y;
            float z = -world.m.y.z;
            TheShaderMgr.SetVConstant((VShaderConstant)(0x3E + i1), Vector4(x, y, z, 0));
            TheShaderMgr.SetPConstant((PShaderConstant)(0x3E + i1), Vector4(x, y, z, 0));
            TheShaderMgr.SetVConstant(
                (VShaderConstant)(0x42 + i1),
                Vector4(color.red, color.green, color.blue, 1)
            );
            TheShaderMgr.SetPConstant(
                (PShaderConstant)(0x42 + i1),
                Vector4(color.red, color.green, color.blue, 1)
            );
            Transform projection = light.Projection();
            TheShaderMgr.SetPConstant4x3(
                (PShaderConstant)(i2 * 3 + 0x5F), Hmx::Matrix4(projection)
            );
            TheShaderMgr.SetPConstant((PShaderConstant)(i2 + 5), light.GetShadowMap());
            TheRenderState.SetTextureFilter(
                i2 + 5, RndRenderState::kFilterModeLinear, false
            );
            if (light.GetProjectedBlend() == 0) {
                TheShaderMgr.SetPConstant((PShaderConstant)(i2 + 10), light.GetTexture());
                TheRenderState.SetTextureClamp(i2 + 10, RndRenderState::kClampModeBorder);
                TheRenderState.SetBorderColor(i2 + 10, false);
                TheRenderState.SetTextureFilter(
                    i2 + 10, RndRenderState::kFilterModeLinear, false
                );
                TheRenderState.SetTextureClamp(i2 + 5, RndRenderState::kClampModeBorder);
                TheRenderState.SetBorderColor(i2 + 5, true);
            } else if (light.GetProjectedBlend() == 1) {
                TheShaderMgr.SetPConstant(
                    (PShaderConstant)(i2 + 10),
                    TheRnd.GetDefaultTex(Rnd::kDefaultTex_White)
                );
                TheRenderState.SetTextureClamp(i2 + 5, RndRenderState::kClampModeBorder);
                TheRenderState.SetBorderColor(i2 + 5, false);
            }
            return true;
        }
        return false;
    }

}

NgEnviron::NgEnviron()
    : mProjectedBlend(), mNumLightsReal(0), mNumLightsApprox(0), mNumLightsPoint(0),
      mNumLightsProj(0), mHasPointCubeTex(0) {}

void NgEnviron::Select(const Vector3 *vptr) {
    mNumLightsReal = 0;
    mNumLightsApprox = 0;
    mNumLightsPoint = 0;
    mNumLightsProj = 0;
    mHasPointCubeTex = false;
    mProjectedBlend = (RndLight::ProjectedBlend)0;
    if (TheRnd.DrawMode() != 4 && TheRnd.DrawMode() != 2 && TheRnd.DrawMode() != 6
        && TheRnd.DrawMode() != 3) {
        ReclassifyLights();
        int pointLightCount = 0;
        int projLightCount = 0;
        NgLight *pointLights[3];
        NgLight *projLights[1];
        FOREACH (it, mLightsReal) {
            NgLight *cur = static_cast<NgLight *>(*it);
            switch (cur->GetType()) {
            case RndLight::kPoint:
                if (pointLightCount < 3 && CheckPointLight(*cur)) {
                    pointLights[pointLightCount] = cur;
                    pointLightCount++;
                }
                break;
            case RndLight::kFakeSpot:
                if (projLightCount < 1 && CheckProjLight(*cur)) {
                    if (projLightCount == 0) {
                        mProjectedBlend =
                            (RndLight::ProjectedBlend)cur->GetProjectedBlend();
                    } else if (mProjectedBlend != cur->GetProjectedBlend()) {
                        MILO_NOTIFY(
                            "%s: projected light has different blend mode than another light already in the environment (%s)",
                            cur->Name(),
                            PathName(this)
                        );
                    }
                    projLights[projLightCount] = cur;
                    projLightCount++;
                }
                break;
            default:
                MILO_NOTIFY_ONCE("%s: Invalid real light", PathName(cur));
                break;
            }
        }
        RndEnviron::Select(vptr);
        ClearPointCubeTex();
        ClearLightTransforms();
        for (unsigned int i = 0; i < 4; i++) {
            ClearLightRegisters(i);
        }
        for (int i = 0; i < projLightCount; i++) {
            if (SetProjLightRegisters(3 - i, i, *projLights[i])) {
                mNumLightsProj++;
                mNumLightsReal++;
            }
        }
        for (int i = 0; i < pointLightCount; i++) {
            bool b3;
            if (SetPointLightRegisters(mNumLightsPoint, *pointLights[i], b3)) {
                mNumLightsPoint++;
                mNumLightsReal++;
                if (b3) {
                    mHasPointCubeTex = true;
                }
            }
        }
        UpdateApproxLighting(vptr);
        NgMat::SetCurrent(nullptr);
        TheNgStats->mLightsReal += mNumLightsReal;
        TheNgStats->mLightsApprox += mNumLightsApprox;
        if (FogEnable()) {
            float fogEnd = GetFogEnd();
            TheShaderMgr.SetVConstant(
                (VShaderConstant)0x5B, Vector4(fogEnd, 1 / (fogEnd - GetFogStart()), 0, 1)
            );
            const Hmx::Color &fogColor = FogColor();
            TheShaderMgr.SetPConstant(
                (PShaderConstant)0x5A,
                Vector4(fogColor.red, fogColor.green, fogColor.blue, fogColor.alpha)
            );
        } else {
            TheShaderMgr.SetVConstant((VShaderConstant)0x5B, Vector4(0, 0, 0, 1));
            TheShaderMgr.SetPConstant((PShaderConstant)0x5A, Vector4(0, 0, 0, 0));
        }

        bool b10 = mFadeOut && mFadeEnd != mFadeStart;
        if (b10) {
            Vector4 v130;
            v130.x = mFadeEnd;
            float fadeRange = (mFadeEnd - mFadeStart);
            if (0.001f <= fadeRange || fadeRange <= -0.001f) {
                v130.y = 1 / fadeRange;
            } else {
                v130.y = fadeRange < 0 ? -999.99994f : 999.99994f;
            }
            v130.z = mFadeMax;
            v130.w = 0;
            TheShaderMgr.SetVConstant((VShaderConstant)0x37, v130);
            TheShaderMgr.SetPConstant((PShaderConstant)0x37, v130);
            Vector4 curFade = mLRFade;
            Transform fadeRef = LRFadeRef();
            Vector3 v160 = fadeRef.m.x;
            Normalize(v160, v160);
            Vector4 v170(0, 0, 0, 1);
            float dot = Dot(v160, fadeRef.v);
            if (curFade.x != curFade.y) {
                float scalar = 1 / (curFade.y - curFade.x);
                v170.x = v160.x * scalar;
                v170.y = v160.y * scalar;
                v170.z = v160.z * scalar;
                v170.w = -((curFade.x + dot) * scalar);
            }
            Vector4 v180(0, 0, 0, 1);
            if (curFade.z != curFade.w) {
                float scalar = 1 / (curFade.z - curFade.w);
                v180.x = v160.x * scalar;
                v180.y = v160.y * scalar;
                v180.z = v160.z * scalar;
                v180.w = -((curFade.w + dot) * scalar);
            }
            TheShaderMgr.SetVConstant((VShaderConstant)0x35, v170);
            TheShaderMgr.SetVConstant((VShaderConstant)0x36, v180);
            TheShaderMgr.SetPConstant((PShaderConstant)0x35, v170);
            TheShaderMgr.SetPConstant((PShaderConstant)0x36, v180);
        }
        if (mUseColorAdjust) {
            const Transform &colorXfm = ColorXfm();
            TheShaderMgr.SetPConstant((PShaderConstant)0x6D, Hmx::Matrix4(colorXfm));
        }
        if (mAOEnabled) {
            TheShaderMgr.SetVConstant(
                (VShaderConstant)0x18,
                Vector4(mAOStrength, mAOStrength, mAOStrength, mAOStrength)
            );
        }
        if (mUseToneMapping) {
            TheShaderMgr.SetPConstant(
                (PShaderConstant)0x7C,
                Vector4(mIntensityAverage, mExposure, mWhitePoint, 0)
            );
        }
    } else {
        RndEnviron::Select(vptr);
        NgMat::SetCurrent(nullptr);
        TheNgStats->mLightsReal += mNumLightsReal;
        TheNgStats->mLightsApprox += mNumLightsApprox;
    }
}

void NgEnviron::UpdateApproxLighting(const Vector3 *vptr) {
    mNumLightsApprox = 0;
    bool b2 = mLightsReal.size() || mLightsApprox.size();
    bool b1 = mUseApprox_Local || mUseApprox_Global;

    if (b1 && b2) {
        static BoxMapLighting boxLight;
        static Hmx::Color boxResults[6];
        for (int i = 0; i < 6; i++) {
            boxResults[i].red = 0;
            boxResults[i].green = 0;
            boxResults[i].blue = 0;
        }
        if (mUseApprox_Local) {
            boxLight.Clear();
            FOREACH (it, mLightsApprox) {
                if (boxLight.QueueLight(*it, 1)) {
                    mNumLightsApprox++;
                }
            }
            boxLight.ApplyQueuedLights(boxResults, vptr);
        }
        if (mUseApprox_Global) {
            int numInQueue = RndEnviron::GetGlobalLighting().NumQueuedLights();
            if (numInQueue != 0) {
                mNumLightsApprox += numInQueue;
                RndEnviron::GetGlobalLighting().ApplyQueuedLights(boxResults, vptr);
            }
        }
        for (int i = 0; i < 6; i++) {
            const Hmx::Color &cur = boxResults[i];
            TheShaderMgr.SetVConstant(
                (VShaderConstant)(0x50 + i),
                Vector4(cur.red, cur.green, cur.blue, cur.alpha)
            );
            TheShaderMgr.SetPConstant(
                (PShaderConstant)(0x50 + i),
                Vector4(cur.red, cur.green, cur.blue, cur.alpha)
            );
        }
    } else {
        static Hmx::Color sBlank(0, 0, 0, 0);
        for (int i = 0; i < 6; i++) {
            TheShaderMgr.SetVConstant(
                (VShaderConstant)(0x50 + i),
                Vector4(sBlank.red, sBlank.green, sBlank.blue, sBlank.alpha)
            );
            TheShaderMgr.SetPConstant(
                (PShaderConstant)(0x50 + i),
                Vector4(sBlank.red, sBlank.green, sBlank.blue, sBlank.alpha)
            );
        }
    }
    if (mNumLightsReal > 0) {
        mNumLightsApprox = Max(1, mNumLightsApprox);
    }
}
