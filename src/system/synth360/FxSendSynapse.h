#pragma once
#include "synth/FxSendSynapse.h"
#include "synth360/FxSend.h"

class FxSendSynapse360 : public FxSendSynapse, public FxSend360 {
public:
    FxSendSynapse360() : FxSend360(this) {}
    OBJ_CLASSNAME(FxSendSynapse360)
    OBJ_SET_TYPE(FxSendSynapse360)
    virtual void Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }
    virtual void UpdateMix() { UpdateVolumes(); }
    virtual void OnParametersChanged() { FxSend360::SyncEffectParams(); }
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendSynapse360)

protected:
    virtual IUnknown *CreateFx();
};
