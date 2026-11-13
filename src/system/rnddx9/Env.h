#pragma once
#include "obj/Object.h"
#include "rndobj/Env_NG.h"

class DxEnviron : public NgEnviron {
public:
    OBJ_CLASSNAME(Environ);
    OBJ_SET_TYPE(Environ);
    virtual void Select(const Vector3 *);

    NEW_OBJ(DxEnviron)
    REGISTER_OBJ(DxEnviron)
protected:
    DxEnviron() {}
};
