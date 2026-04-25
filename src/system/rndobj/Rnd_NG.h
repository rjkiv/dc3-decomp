#pragma once
#include "OcclusionQueryMgr.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "rndobj/Cam.h"
#include "rndobj/Flare.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/SoftParticleBuffer.h"
#include "rndobj/Tex.h"

struct LargeQuadRenderData;

class NgRnd : public Rnd {
public:
    // size 0x18
    struct Viewport {
        Viewport() : unk0(0), unk4(0), unk8(0), unkc(0), unk10(0), unk14(0) {}
        int unk0; // x
        int unk4; // y
        int unk8; // width
        int unkc; // height
        float unk10; // minz
        float unk14; // maxz
    };
    struct RndPointTest {
        RndFlare *unk0;
        unsigned int unk4;
        unsigned int unk8;
    };

    NgRnd();
    virtual ~NgRnd();

    virtual void PreInit();
    virtual void Init();
    virtual void ReInit();
    virtual void Terminate();
    virtual void Clear(unsigned int, const Hmx::Color &) {}
    virtual void SetShadowMap(RndTex *, RndCam *, const Hmx::Color *);
    virtual void RemovePointTest(RndFlare *);
    virtual RndTex *GetShadowMap() { return mShadowMap; }
    virtual RndCam *GetShadowCam() { return mShadowCam; }
    virtual void DoPostProcess();

    virtual void SetViewport(const Viewport &v) { unk1e0 = v; }
    virtual const Viewport &GetViewport() const { return unk1e0; }
    virtual void DrawRect(
        const Hmx::Rect &,
        RndMat *,
        ShaderType,
        const Hmx::Color &,
        const Hmx::Color *,
        const Hmx::Color *
    ) {}
    virtual void DrawRectDepth(
        const Vector3 &, const Vector3 (&)[4], const Vector4 &, RndMat *, ShaderType
    ) {}
    virtual bool Offscreen() const { return false; }
    virtual RndTex *PreProcessTexture() { return nullptr; } // 0x12c
    virtual RndTex *PostProcessTexture() { return nullptr; }
    virtual RndTex *PreDepthTexture() { return nullptr; }
    virtual void Suspend() {} // 0x138
    virtual void Resume() {} // 0x13c
    virtual RndSoftParticleBuffer *ParticleBuffer() { return unk208; }
    virtual void CreateLargeQuad(int, int, LargeQuadRenderData &);
    virtual void
    DrawLargeQuad(const LargeQuadRenderData &, const Transform &, RndMat *, ShaderType);
    virtual void SetVertShaderTex(RndTex *, int);

protected:
    virtual void ResetStats();
    virtual float UpdateOverlay(RndOverlay *, float);

    Viewport unk1e0;
    bool unk1f8;
    RndTex *mShadowMap; // 0x1fc
    RndCam *mShadowCam; // 0x200
    RndOcclusionQueryMgr *mOcclusionQueryMgr; // 0x204
    RndSoftParticleBuffer *unk208; // 0x208
    std::vector<RndPointTest> unk20c; // 0x20c
    bool unk218; // 0x218
};

extern NgRnd &TheNgRnd;
