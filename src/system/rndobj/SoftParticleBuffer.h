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
    void Queue(RndDrawable *, BaseMaterial::Blend);

private:
    void AllocateData(unsigned int, unsigned int, unsigned int);
    void FreeData();
    void BlurSurface();

    RndTex *mSurfaces[2]; // 0x30
    BaseMaterial::Blend unk38; // 0x38
    ObjPtrList<RndDrawable> unk3c; // 0x3c
};
