#pragma once
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Lit.h"
#include "rndobj/Tex.h"

class NgLight : public RndLight {
public:
    OBJ_CLASSNAME(Light)
    OBJ_SET_TYPE(Light)
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual ~NgLight();

    NEW_OBJ(NgLight);

    void CheckShadowMap();

    static void Init();
    static void Terminate();

protected:
    NgLight();
    virtual void RenderShadows(std::vector<RndDrawable *> &);
    virtual void SetAndClearShadowViewport();
    virtual void BlurShadowRT();

    bool WantShadows() const;
    bool SphereConeTest(const Vector3 &, float);
    RndTex *CreateShadowTex();
    void SetShadowTransforms();
    bool HaveShadows(std::vector<RndDrawable *> &);

    RndTex *mShadowRT; // 0x180
    int mShadowMap; // 0x184
    RndTex *unk188; // 0x188
    int unk18c; // 0x18c
};
