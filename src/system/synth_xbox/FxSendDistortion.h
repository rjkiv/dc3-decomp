#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendDistortion.h"
#include "xdk/xapilibi/xbase.h"

class FxSendDistortion360 : public FxSendDistortion, public FxSend360 {
public:
    virtual ~FxSendDistortion360();
    OBJ_CLASSNAME(FxSendDistortion360)
    OBJ_SET_TYPE(FxSendDistortion360)

    NEW_OBJ(FxSendDistortion360)

    FxSendDistortion360();

protected:
    virtual IUnknown *CreateFx();
};
