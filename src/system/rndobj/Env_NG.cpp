#include "rndobj/Env_NG.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "rndobj/Lit_NG.h"
#include "rndobj/RenderState.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"

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
