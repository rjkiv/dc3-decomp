#pragma once
#include "obj/Object.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Draw.h"
#include "rndobj/PostProc.h"
#include "rndobj/Tex.h"

class RndSoftParticleBuffer : public Hmx::Object, public PostProcessor {
public:
    RndSoftParticleBuffer();
    virtual ~RndSoftParticleBuffer();
    // Hmx::Object
    OBJ_CLASSNAME(SoftParticleBuffer);
    OBJ_SET_TYPE(SoftParticleBuffer);
    // PostProcessor
    virtual void DoPost();
    virtual const char *GetProcType() { return "SoftParticleBuffer"; }

    NEW_OBJ(RndSoftParticleBuffer)
    REGISTER_OBJ(RndSoftParticleBuffer)
    void Queue(RndDrawable *draw, BaseMaterial::Blend blendMode);

private:
    void AllocateData(unsigned int width, unsigned int height, unsigned int bpp);
    void FreeData();
    void BlurSurface();

    // private: static const uint32_t kNumSurfaces;

    RndTex *mSurfaces[2]; // 0x30
    BaseMaterial::Blend mBlendMode; // 0x38
    ObjPtrList<RndDrawable> mQueuedObjects; // 0x3c
};
