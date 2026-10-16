#pragma once
#include "obj/Object.h"
#include "rndobj/Env.h"
#include "rndobj/Lit.h"

class NgEnviron : public RndEnviron {
public:
    // Hmx::Object
    OBJ_CLASSNAME(NgEnviron);
    OBJ_SET_TYPE(NgEnviron);
    // RndEnviron
    virtual void Select(const Vector3 *);
    virtual void UpdateApproxLighting(const Vector3 *);
    virtual int NumLights_Real() const { return mNumLights_Real; }
    virtual int NumLights_Approx() const { return mNumLights_Approx; }
    virtual int NumLights_Point() const { return mNumLights_Point; }
    virtual int NumLights_Proj() const { return mNumLights_Proj; }
    virtual bool HasPointCubeTex() const { return mHasPointCubeTex; }
    virtual RndLight::ProjectedBlend GetProjectedBlend() const { return mProjectedBlend; }

    NEW_OBJ(NgEnviron)

protected:
    NgEnviron();

    RndLight::ProjectedBlend mProjectedBlend; // 0x298
    int mNumLights_Real; // 0x29c
    int mNumLights_Approx; // 0x2a0
    int mNumLights_Point; // 0x2a4
    int mNumLights_Proj; // 0x2a8
    bool mHasPointCubeTex; // 0x2ac
};
