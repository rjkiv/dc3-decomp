#pragma once
#include "obj/Object.h"
#include "rnddx9/Object.h"
#include "rndobj/TexRenderer.h"

class DxTexRenderer : public RndTexRenderer, public DxObject {
public:
    // Hmx::Object
    OBJ_CLASSNAME(TexRenderer);
    OBJ_SET_TYPE(TexRenderer);
    virtual void PostDeviceReset() { mDirty = true; }

    NEW_OBJ(DxTexRenderer)
    REGISTER_OBJ(DxTexRenderer)
};
