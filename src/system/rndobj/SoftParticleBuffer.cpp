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
    work->SetZMode(BaseMaterial::kDisable);
    work->SetTexWrap(BaseMaterial::kClamp);

    for (unsigned int i = 0; i < 2; i++) {
        RndTex *curSurface = mSurfaces[i - 1 & 1];
        work->SetDiffuseTex(mSurfaces[i & 1]);
        curSurface->MakeDrawTarget();

        float w = curSurface->Width();
        float h = curSurface->Height();
        float invw = 1 / w;
        float invh = 1 / h;
        Hmx::Rect blurRect(0, 0, w, h);

        static const float kBaseOffset = 0.5f;
        static const float kBlurWeightOffset[5][2] = {
            { 0.1f,  kBaseOffset * -3 },
            { 0.25f, kBaseOffset * -1 },
            { 0.3f,  kBaseOffset      },
            { 0.25f, kBaseOffset * 3  },
            { 0.1f,  kBaseOffset * 5  }
        };

        bool b1 = !(i & 1);
        for (unsigned int j = 0; j < 5; j++) {
            float f20 = kBlurWeightOffset[j][0];
            float blurY = kBlurWeightOffset[j][1];
            float x, y;

            if (b1) {
                x = blurY * invw;
                y = invh * kBaseOffset;
            } else {
                x = invw * kBaseOffset;
                y = blurY * invh;
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
