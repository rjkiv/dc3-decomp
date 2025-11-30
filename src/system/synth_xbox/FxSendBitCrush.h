#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendBitCrush.h"
#include "xdk/xapilibi/xbase.h"

class FxSendBitCrush360 : public FxSendBitCrush, public FxSend360 {
public:
    virtual ~FxSendBitCrush360();
    OBJ_CLASSNAME(FxSendBitCrush360)
    OBJ_SET_TYPE(FxSendBitCrush360)

    NEW_OBJ(FxSendBitCrush360)

    FxSendBitCrush360();

protected:
    virtual IUnknown *CreateFx();
};
