#pragma once
#include "synth/FxSendPitchShift.h"
#include "synth360/FxSend.h"

class FxSendPitchShift360 : public FxSendPitchShift, public FxSend360 {
public:
    FxSendPitchShift360() : FxSend360(this) {}
    OBJ_CLASSNAME(FxSendPitchShift360)
    OBJ_SET_TYPE(FxSendPitchShift360)
    virtual void Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }
    virtual void UpdateMix() { UpdateVolumes(); }
    virtual void OnParametersChanged() { FxSend360::SyncEffectParams(); }
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendPitchShift360)

protected:
    virtual IUnknown *CreateFx();
};
