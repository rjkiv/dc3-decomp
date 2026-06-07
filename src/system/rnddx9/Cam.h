#pragma once
#include "obj/Object.h"
#include "rnddx9/Object.h"
#include "rndobj/Cam.h"

class DxCam : public RndCam, public DxObject {
public:
    OBJ_CLASSNAME(Cam);
    OBJ_SET_TYPE(Cam);
    virtual void Select();
    virtual unsigned int ProjectZ(float);
    virtual void PostDeviceReset() { UpdateLocal(); }

    NEW_OBJ(DxCam)

    void SetViewport();

protected:
    DxCam();
};
