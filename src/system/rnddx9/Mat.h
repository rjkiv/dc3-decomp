#pragma once

#include "obj/Object.h"
#include "rndobj/Mat_NG.h"

class DxMat : public NgMat {
public:
    DxMat() {}
    virtual ~DxMat() {}
    OBJ_CLASSNAME(Mat)
    OBJ_SET_TYPE(DxMat)

    NEW_OBJ(DxMat)
    REGISTER_OBJ(DxMat)

    static void Init();
};
