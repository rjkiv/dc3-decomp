#include "rndobj/SoftParticleBuffer.h"
#include "Rnd_NG.h"
#include "math/Geo.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/RenderState.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"

RndSoftParticleBuffer::RndSoftParticleBuffer()
    : mBlendMode(BaseMaterial::kBlendSrcAlphaAdd), mQueuedObjects(this) {
    for (int i = 0; i < 2; i++) {
        mSurfaces[i] = nullptr;
    }
    int width = TheNgRnd.Width() >> 2;
    int height = TheNgRnd.Height() >> 2;
    AllocateData(width, height, TheNgRnd.Bpp());
}

RndSoftParticleBuffer::~RndSoftParticleBuffer() { FreeData(); }

void RndSoftParticleBuffer::DoPost() {
    TheShaderMgr.SetPostSoftDepth(false);
    if (!mQueuedObjects.empty()) {
        if (TheNgRnd.PreDepthTexture() && mSurfaces[0]) {
            RndCam *curCam = RndCam::Current();
            RndCam *cam = TheRnd.GetWorldCamCopy();
            cam->SetTargetTex(mSurfaces[0]);
            cam->Select();
            Rnd::Mode mode = TheRnd.DrawMode();
            TheRnd.SetDrawMode(Rnd::kDrawSoftDepth);
            TheShaderMgr.SetPConstant(kPShader_DepthHalf, TheNgRnd.PreDepthTexture());
            TheRenderState.SetTextureFilter(
                kPShader_DepthHalf, RndRenderState::kPoint, false
            );
            TheRenderState.SetTextureClamp(kPShader_DepthHalf, RndRenderState::kClamp);
            Vector4 values;
            cam->GetDepthRangeValues(values);
            TheShaderMgr.SetPConstant(kPShader_ZRange, values);
            FOREACH (it, mQueuedObjects) {
                (*it)->Draw();
            }
            TheRnd.SetDrawMode(mode);
            cam->SetTargetTex(nullptr);
            curCam->Select();
            BlurSurface();
            TheShaderMgr.SetPostSoftDepth(true);
            TheShaderMgr.SetPConstant(kPShader_SoftDepthMap, mSurfaces[0]);
            TheRenderState.SetTextureFilter(
                kPShader_SoftDepthMap, RndRenderState::kLinear, false
            );
            TheRenderState.SetTextureClamp(kPShader_SoftDepthMap, RndRenderState::kClamp);
        }
    }
    mQueuedObjects.clear();
}

void RndSoftParticleBuffer::FreeData() {
    TheNgRnd.UnregisterPostProcessor(this);
    for (int i = 0; i < 2; i++) {
        RELEASE(mSurfaces[i]);
    }
}

void RndSoftParticleBuffer::AllocateData(
    unsigned int w, unsigned int h, unsigned int bpp
) {
    if (w && h && bpp) {
        for (int i = 0; i < 2; i++) {
            MILO_ASSERT(mSurfaces[i] == NULL, 0xC6);
            mSurfaces[i] = Hmx::Object::New<RndTex>();
            mSurfaces[i]->SetBitmap(w, h, bpp, RndTex::kRenderedNoZ, false, nullptr);
        }
    }
    TheNgRnd.RegisterPostProcessor(this);
}

void RndSoftParticleBuffer::Queue(RndDrawable *draw, BaseMaterial::Blend blend) {
    if (blend == mBlendMode) {
        if (mQueuedObjects.find(draw) == mQueuedObjects.end()) {
            mQueuedObjects.push_back(draw);
        }
    }
}

void RndSoftParticleBuffer::BlurSurface() {
    RndMat *work = TheShaderMgr.GetWork();
    work->SetBlend(BaseMaterial::kBlendSrc);
    work->SetZMode(kZModeDisable);
    work->SetTexWrap(kTexWrapClamp);
    for (unsigned int i = 0; i < 2; i++) {
        RndTex *curSurface = mSurfaces[i - 1 & 1];
        work->SetDiffuseTex(mSurfaces[i & 1]);
        curSurface->MakeDrawTarget();
        Hmx::Rect blurRect(1, 1, curSurface->Width(), curSurface->Height());
        float invw = 1 / blurRect.w;
        float invh = 1 / blurRect.h;

        static const float kBaseOffset = 0.5f;
        // pdb says this is a float [5][2], but maybe they made it a Vector2 later on
        static const Vector2 kBlurWeightOffset[5] = { Vector2(0.1f, -1.5f),
                                                      Vector2(0.25f, -0.5f),
                                                      Vector2(0.3f, 0.5f),
                                                      Vector2(0.25f, 1.5f),
                                                      Vector2(0.1f, 2.5f) };

        bool b1 = !(i & 1);
        for (unsigned int j = 0; j < 5; j++) {
            float f20 = kBlurWeightOffset[j].x;
            float x, y;

            if (b1) {
                x = kBlurWeightOffset[j].y * invw;
                y = kBaseOffset * invh;
            } else {
                x = kBaseOffset * invh;
                y = kBlurWeightOffset[j].y * invw;
            }
            TheShaderMgr.SetPConstant(
                (PShaderConstant)(kPShader_BlurSampleOffsets + j), Vector4(x, y, 1, 1)
            );
            TheShaderMgr.SetPConstant(
                (PShaderConstant)(kPShader_BlurSampleWeights + j),
                Vector4(f20, f20, f20, f20)
            );
        }
        TheShaderMgr.SetNumTaps(5);
        TheNgRnd.DrawRect(blurRect, work, kBlurShader, Hmx::Color(), nullptr, nullptr);
        TheShaderMgr.SetNumTaps(1);
        curSurface->FinishDrawTarget();
    }
}
