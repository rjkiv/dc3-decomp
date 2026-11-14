#pragma once
#include "obj/Object.h"
#include "rnddx9/Object.h"
#include "rndobj/Lit.h"
#include "rndobj/Lit_NG.h"

class DxLight : public NgLight, public DxObject {
public:
    virtual ~DxLight() {}
    OBJ_CLASSNAME(Light)
    OBJ_SET_TYPE(Light)
    virtual void Copy(const Hmx::Object *o, CopyType ty) { NgLight::Copy(o, ty); }
    virtual void Load(BinStream &bs) { NgLight::Load(bs); }
    virtual void SetColor(const Hmx::Color &c) { RndLight::SetColor(c); }
    virtual void SetLightType(Type t) { RndLight::SetLightType(t); }
    virtual void SetRange(float r) { RndLight::SetRange(r); }

    static void Init();
    static void Terminate();
    NEW_OBJ(DxLight)
    REGISTER_OBJ(DxLight)

protected:
    DxLight();
};
