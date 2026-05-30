#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendChorus.h"

class FxSendChorus360 : public FxSendChorus, public FxSend360 {
public:
    FxSendChorus360();
    virtual ~FxSendChorus360();
    OBJ_CLASSNAME(FxSendChorus360)
    OBJ_SET_TYPE(FxSendChorus360)
    virtual void Recreate(std::vector<FxSend *> &);
    virtual void UpdateMix();
    virtual void OnParametersChanged();
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendChorus360)

protected:
    virtual IUnknown *CreateFx();
};
