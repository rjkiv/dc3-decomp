#include "rnddx9/Part.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Stats_NG.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/win_types.h"
#include <cstddef>

DxParticleSys::DxParticleSys() {}

struct DxParticleVertex {
    float posx, posy, posz; // broken out. thanks alignment
    int color;
    Vector4 uv0;
    float uv1;
};
// clang-format off
static D3DVERTEXELEMENT9 sElement[5] = {
    { 0,
      offsetof(DxParticleVertex, posx),
      D3DDECLTYPE_FLOAT3,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_POSITION,
      0 },
    { 0,
      offsetof(DxParticleVertex, color),
      D3DDECLTYPE_D3DCOLOR,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_COLOR,
      0 },
    { 0,
      offsetof(DxParticleVertex, uv0),
      D3DDECLTYPE_FLOAT4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_TEXCOORD,
      0 },
    { 0,
      offsetof(DxParticleVertex, uv1),
      D3DDECLTYPE_FLOAT1,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_TEXCOORD,
      1 },
    D3DDECL_END()
};
// clang-format on

void DxParticleSys::Init() {
    REGISTER_OBJ_FACTORY(DxParticleSys);
    MILO_ASSERT(!sVertexDecl, 0x46);
    HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(sElement, &sVertexDecl);
    DX_ASSERT(hr, 0x47);
}

void DxParticleSys::DrawShowing() {
    RndParticleSys::DrawShowing();
    if (mActiveParticles != nullptr) {
        Hmx::Color particle_color;
        Hmx::Matrix3 inv_rel_mtx;
        particle_color.ResetFake();
        Invert(mRelativeXfm.m, inv_rel_mtx);
        Multiply(RndCam::Current()->WorldXfm().m, inv_rel_mtx, inv_rel_mtx);
        bool is_fancy = mType == kFancy;
        inv_rel_mtx.x *= 0.5f;
        inv_rel_mtx.y *= 0.5f;
        inv_rel_mtx.z *= 0.5f * mScreenAspect;
        is_fancy &= mStretchWithVelocity;
        bool align_with_vel = mAlignWithVelocity == 0;
        bool constant_area = mConstantArea;

        TheShaderMgr.SetVConstant(
            (VShaderConstant)0x31,
            Vector4(
                is_fancy ? 1.0f : 0.0f,
                align_with_vel ? 1.0f : 0.0f,
                constant_area ? 1.0f : 0.0f,
                mStretchScale * 2.0f
            )
        );
        TheShaderMgr.SetVConstant(
            (VShaderConstant)0x2f,
            Vector4(
                inv_rel_mtx.x.x,
                inv_rel_mtx.x.y,
                inv_rel_mtx.x.z,
                is_fancy && align_with_vel && constant_area && mPerspectiveStretch ? 0 : 1
            )
        );
        TheShaderMgr.SetVConstant(
            (VShaderConstant)0x30,
            Vector4(inv_rel_mtx.z.x, inv_rel_mtx.z.y, inv_rel_mtx.z.z, 1.0f)
        );
        TheShaderMgr.SetVConstant(
            (VShaderConstant)0x32,
            Vector4(
                mNumTilesAcross,
                mNumTilesDown,
                1.0f / mNumTilesAcross,
                1.0f / mNumTilesDown
            )
        );
        float do_uv_anim = mAnimateUVs ? 1.0f : 0.0f;
        TheShaderMgr.SetVConstant(
            (VShaderConstant)0x33, Vector4(do_uv_anim, do_uv_anim, do_uv_anim, do_uv_anim)
        );
        TheShaderMgr.SetTransform(mRelativeXfm);
        RndShader::SelectConfig(mMat, kParticlesShader, false);
        DrawParticles(particle_color);
    }
}

void DxParticleSys::DrawParticles(const Hmx::Color &col) {
    if (mNumActive == 0) {
        return;
    }
    MILO_ASSERT(mActiveParticles, 551);
    int num_drawn = mNumActive;
    TheDxRnd.Device()->SetVertexDeclaration(sVertexDecl);
    HRESULT hr = TheDxRnd.Device()->BeginVertices(
        D3DPT_QUADLIST, num_drawn * 4, sizeof(DxParticleVertex), nullptr
    );
    DX_ASSERT(hr, 559);

    bool align = mAlignWithVelocity;
    if (mActiveParticles != nullptr) {
        DxParticleVertex *upload = (DxParticleVertex *)hr;
        RndParticle *particle = mActiveParticles;
        if (align) {
            int c = col.PackAlpha();
            upload->posx = particle->pos.x;
            upload->posy = particle->pos.y;
            upload->posz = particle->pos.z;
            upload->color = c;
            upload->uv0.x = particle->vel.x * 2;
            upload->uv0.y = particle->vel.y * 2;
            upload->uv0.z = particle->vel.z * 2;
            upload->uv1 = particle->tileIdx;
            particle = particle->next;
            upload++;
        } else {
            while (particle != nullptr) {
                int c = col.PackAlpha();
                upload->posx = particle->pos.x;
                upload->posy = particle->pos.y;
                upload->posz = particle->pos.z;
                upload->color = c;
                upload->uv0.x = particle->size;
                upload->uv0.y = particle->angle;
                upload->uv0.z = particle->swingArm;
                upload->uv1 = particle->tileIdx;
                particle = particle->next;
                upload++;
            }
        }
    }
    DxParticleVertex *verts = (DxParticleVertex *)hr;
    TheDxRnd.Device()->EndVertices();
    TheNgStats->mParts += num_drawn;
    TheNgStats->mPartSys += (num_drawn != 0);
}
