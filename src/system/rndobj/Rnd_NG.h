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
        Viewport() : mX(0), mY(0), mWidth(0), mHeight(0), mMinZ(0), mMaxZ(0) {}
        Viewport(int x, int y, int w, int h, float minZ, float maxZ)
            : mX(x), mY(y), mWidth(w), mHeight(h), mMinZ(minZ), mMaxZ(maxZ) {}
        int mX; // 0x0
        int mY; // 0x4
        int mWidth; // 0x8
        int mHeight; // 0xc
        float mMinZ; // 0x10
        float mMaxZ; // 0x14
    };
    struct RndPointTest {
        RndFlare *mFlare; // 0x0
        unsigned int mQueryIndexPoint; // 0x4
        unsigned int mQueryIndexArea; // 0x8
    };
    enum ClearFlags {
        kColor = 0x0001,
        kDepth = 0x0002,
    };

    NgRnd();
    virtual ~NgRnd();
    virtual void PreInit();
    virtual void Init();
    virtual void ReInit();
    virtual void Terminate();
    virtual void Clear(unsigned int clearFlags, const Hmx::Color &color) {}
    virtual void
    SetShadowMap(RndTex *shadowMap, RndCam *shadowCam, const Hmx::Color *color);
    virtual void RemovePointTest(RndFlare *);
    virtual RndTex *GetShadowMap() { return mShadowMap; }
    virtual RndCam *GetShadowCam() { return mShadowCam; }
    virtual void DoPostProcess();

    virtual void SetViewport(const Viewport &vp) { mCurrentVP = vp; }
    virtual const Viewport &GetViewport() const { return mCurrentVP; }
    virtual void DrawRect(
        const Hmx::Rect &rect,
        RndMat *mat,
        ShaderType shader,
        const Hmx::Color &c,
        const Hmx::Color *right,
        const Hmx::Color *bottom
    ) {}
    virtual void DrawRectDepth(
        const Vector3 &camPos,
        const Vector3 (&camCorner)[4],
        const Vector4 &depthRange,
        RndMat *mat,
        ShaderType shader
    ) {}
    virtual bool Offscreen() const { return false; }
    virtual RndTex *PreProcessTexture() { return nullptr; } // 0x12c
    virtual RndTex *PostProcessTexture() { return nullptr; }
    virtual RndTex *PreDepthTexture() { return nullptr; }
    virtual void Suspend() {} // 0x138
    virtual void Resume() {} // 0x13c
    virtual RndSoftParticleBuffer *ParticleBuffer() { return mParticleBuffer; }
    virtual void CreateLargeQuad(int width, int height, LargeQuadRenderData &output);
    virtual void DrawLargeQuad(
        const LargeQuadRenderData &data,
        const Transform &xfm,
        RndMat *mat,
        ShaderType shader
    );
    virtual void SetVertShaderTex(RndTex *tex, int index);

protected:
    virtual void ResetStats();
    virtual float UpdateOverlay(RndOverlay *o, float y);

    Viewport mCurrentVP; // 0x1e0
    bool mWideScreenDisplay; // 0x1f8
    RndTex *mShadowMap; // 0x1fc
    RndCam *mShadowCam; // 0x200
    RndOcclusionQueryMgr *mQueryManager; // 0x204
    RndSoftParticleBuffer *mParticleBuffer; // 0x208
    std::vector<RndPointTest> mLastPointTests; // 0x20c
    bool mPreInitialized; // 0x218
};

extern NgRnd &TheNgRnd;
