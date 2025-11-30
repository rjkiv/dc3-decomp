#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendWah.h"

class FxSendWah360 : public FxSendWah, public FxSend360 {
public:
    OBJ_CLASSNAME(FxSendWah360)
    OBJ_SET_TYPE(FxSendWah360)
    virtual void Recreate(std::vector<FxSend *> &);
    virtual void UpdateMix();
    virtual void OnParametersChanged();
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendWah360)

    FxSendWah360();

protected:
    virtual IUnknown *CreateFx();
};
