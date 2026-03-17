#include "rndobj/PostProc_NG.h"
#include "Memory.h"
#include "ShaderMgr.h"
#include "Tex.h"
#include "math/Rand.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/PostProc.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Tex.h"
#include "rndobj/VelocityBuffer.h"
#include "utl/Loader.h"

Hmx::Color NgPostProc::s_prevBloomColor(-1, -1, -1, -1);
float NgPostProc::s_prevBloomIntensity = -1;
NgPostProc::BloomTextures<3> NgPostProc::sBloom;

NgPostProc::BloomTextureSet::BloomTextureSet() : mBloomTexture() {}

NgPostProc::BloomTextureSet::~BloomTextureSet() { FreeTextures(); }

void NgPostProc::BloomTextureSet::AllocateTextures(unsigned int w, unsigned int h) {
    MILO_ASSERT(mBloomTexture[0] == NULL, 0x48);
    mBloomTexture[0] = Hmx::Object::New<RndTex>();
    mBloomTexture[0]->SetBitmap(w, h, TheRnd.Bpp(), RndTex::kRenderedNoZ, false, nullptr);
    mBloomTexture[1] = mBloomTexture[0];
}

void NgPostProc::BloomTextureSet::FreeTextures() { RELEASE(mBloomTexture[0]); }

NgPostProc::NgPostProc()
    : unk22c(RandomFloat()), unk230(RandomFloat()), unk234(0), unk238(0), unk23c(this),
      unk250(1) {}

NgPostProc::~NgPostProc() {}

void NgPostProc::Select() {
    RndPostProc::Select();
    unk22c = RandomFloat();
    unk230 = RandomFloat();
}

void NgPostProc::OnSelect() {
    RndPostProc::OnSelect();
    unk23c.clear();
}

void NgPostProc::OnUnselect() {
    RndPostProc::OnUnselect();
    unk23c.clear();
}

void NgPostProc::Init() {
    REGISTER_OBJ_FACTORY(NgPostProc);
    PhysMemTypeTracker tracker("D3D(phys):NgPostProc");
    RebuildTex();
}

void NgPostProc::RebuildTex() {
    ReleaseTex();
    int w = 0x80;
    int h = 0x80;
    if (TheLoadMgr.GetPlatform() != kPlatformNone) {
        MILO_ASSERT(TheNgRnd.PreProcessTexture(), 0x3AB );
        w = TheNgRnd.PreProcessTexture()->Width();
        h = TheNgRnd.PreProcessTexture()->Height();
    }
    RndVelocityBuffer::Singleton().AllocateData(w, h, TheRnd.Bpp());
    sBloom.AllocateTextures(w * 4, h * 4);
}

void NgPostProc::EndWorld() {
    // scary
    RndVelocityBuffer::Singleton().CacheCameraSettings(TheRnd.GetWorldCamCopy());
}

void NgPostProc::Terminate() { ReleaseTex(); }

void NgPostProc::SetBloomColor() {
    const float max = Max(1.0f, mBloomThreshold);
    const float inverse = 1.0f / max;
    const Vector4 bloom(0.3f * inverse, 0.59f * inverse, 0.11f * inverse, 1.0f / inverse);
    TheShaderMgr.SetPConstant((PShaderConstant)7, bloom);
}

void NgPostProc::CheckGradientMap() {
    if (DoGradientMap()) {
        Vector4 vec(
            mGradientMapStart, mGradientMapEnd, mGradientMapIndex, mGradientMapOpacity
        );
        TheShaderMgr.SetPConstant((PShaderConstant)3, mGradientMap.Ptr());
        TheShaderMgr.SetPConstant((PShaderConstant)118, vec);
        TheShaderMgr.SetUnk3a(mGradientMap.Ptr());
    }
}

void NgPostProc::CheckVignette() {
    if (DoVignette()) {
        Vector4 vec(
            mVignetteColor.red,
            mVignetteColor.green,
            mVignetteColor.blue,
            mVignetteIntensity
        );
        TheShaderMgr.SetPConstant((PShaderConstant)123, vec);
        TheShaderMgr.SetUnk3e(true);
    }
}