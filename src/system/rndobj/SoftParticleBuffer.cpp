#include "rndobj/SoftParticleBuffer.h"
#include "Rnd_NG.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Tex.h"

RndSoftParticleBuffer::RndSoftParticleBuffer() : unk38(4), unk3c(this) {
    for (int i = 0; i < 2; i++) {
        mSurfaces[i] = nullptr;
    }
    AllocateData(TheNgRnd.Width() >> 2, TheNgRnd.Height() >> 2, TheNgRnd.Bpp());
}

RndSoftParticleBuffer::~RndSoftParticleBuffer() { FreeData(); }

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
