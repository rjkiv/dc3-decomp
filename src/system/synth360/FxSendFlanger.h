#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendFlanger.h"
#include "xdk/xaudio2/xaudio2.h"

class FxSendFlanger360 : public FxSendFlanger, public FxSend360 {
public:
    FxSendFlanger360();
    // Hmx::Object
    virtual ~FxSendFlanger360();
    OBJ_CLASSNAME(FxSendFlanger360)
    OBJ_SET_TYPE(FxSendFlanger360)
    virtual void Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }
    virtual void UpdateMix() { UpdateVolumes(); }
    virtual void OnParametersChanged() { FxSend360::SyncEffectParams(); }

    // FxSendFlanger
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendFlanger360)

protected:
    virtual IUnknown *CreateFx();
};
