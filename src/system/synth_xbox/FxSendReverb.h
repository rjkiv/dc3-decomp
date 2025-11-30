#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendReverb.h"

class FxSendReverb360 : public FxSendReverb, public FxSend360 {
public:
    virtual ~FxSendReverb360();
    OBJ_CLASSNAME(FxSendReverb360)
    OBJ_SET_TYPE(FxSendReverb360)
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendReverb360)

    FxSendReverb360();

protected:
    virtual IUnknown *CreateFx();
};
