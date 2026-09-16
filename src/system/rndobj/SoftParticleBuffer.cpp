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
    : unk38(BaseMaterial::kBlendSrcAlphaAdd), unk3c(this) {
    for (int i = 0; i < 2; i++) {
        mSurfaces[i] = nullptr;
    }
    AllocateData(TheNgRnd.Width() >> 2, TheNgRnd.Height() >> 2, TheNgRnd.Bpp());
}

RndSoftParticleBuffer::~RndSoftParticleBuffer() { FreeData(); }

void RndSoftParticleBuffer::DoPost() {
    TheShaderMgr.SetUnk3f(false);
    if (!unk3c.empty()) {
        if (TheNgRnd.PreDepthTexture() && mSurfaces[0]) {
            RndCam *curCam = RndCam::Current();
            RndCam *cam = TheRnd.GetWorldCamCopy();
            cam->SetTargetTex(mSurfaces[0]);
            cam->Select();
            Rnd::Mode mode = TheRnd.DrawMode();
            TheRnd.SetDrawMode((Rnd::Mode)7);
            TheShaderMgr.SetPConstant((PShaderConstant)9, TheNgRnd.PreDepthTexture());
            TheRenderState.SetTextureFilter(9, RndRenderState::kFilterModePoint, false);
            TheRenderState.SetTextureClamp(9, RndRenderState::kClampModeClamp);
            Vector4 values;
            cam->GetDepthRangeValues(values);
            TheShaderMgr.SetPConstant((PShaderConstant)0x59, values);
            FOREACH (it, unk3c) {
                (*it)->Draw();
            }
            TheRnd.SetDrawMode(mode);
            cam->SetTargetTex(nullptr);
            curCam->Select();
            BlurSurface();
            TheShaderMgr.SetUnk3f(true);
            TheShaderMgr.SetPConstant((PShaderConstant)4, mSurfaces[0]);
            TheRenderState.SetTextureFilter(4, RndRenderState::kFilterModeLinear, false);
            TheRenderState.SetTextureClamp(4, RndRenderState::kClampModeClamp);
        }
    }
    unk3c.clear();
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
    if (blend == unk38) {
        if (unk3c.find(draw) == unk3c.end()) {
            unk3c.push_back(draw);
        }
    }
}

void RndSoftParticleBuffer::BlurSurface() {
    RndMat *work = TheShaderMgr.GetWork();
    work->SetBlend(BaseMaterial::kBlendSrc);
    work->SetZMode(kZModeDisable);
    work->SetTexWrap(kTexWrapClamp);
    for (unsigned int i = 0; i < 2; i++) {
        RndTex *curSurface = mSurfaces[(i - 1) & 1];
        work->SetDiffuseTex(mSurfaces[i & 1]);
        curSurface->MakeDrawTarget();
        float w = curSurface->Width();
        float h = curSurface->Height();
        float invw = 1 / w;
        float invh = 1 / h;
        Hmx::Rect rect(1, 1, w, h);

        static Vector2 sVecs[] = { Vector2(0.1f, -1.5f),
                                   Vector2(0.25f, -0.5f),
                                   Vector2(0.3f, 0.5f),
                                   Vector2(0.25f, 1.5f),
                                   Vector2(0.1f, 2.5f) };

        bool b1 = !(i & 1);
        for (unsigned int j = 0; j < 5; j++) {
            float f20 = sVecs[j].x;
            float x, y;

            if (b1) {
                x = sVecs[j].y * invw;
                y = 0.5f * invh;
            } else {
                x = 0.5f * invh;
                y = sVecs[j].y * invw;
            }
            TheShaderMgr.SetPConstant((PShaderConstant)(0x8A + j), Vector4(x, y, 1, 1));
            TheShaderMgr.SetPConstant(
                (PShaderConstant)(0x9A + j), Vector4(f20, f20, f20, f20)
            );
        }
        TheShaderMgr.SetNumTaps(5);
        TheNgRnd.DrawRect(rect, work, kBlurShader, Hmx::Color(), nullptr, nullptr);
        TheShaderMgr.SetNumTaps(1);
        curSurface->FinishDrawTarget();
    }
}
