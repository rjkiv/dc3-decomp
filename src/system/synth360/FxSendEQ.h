#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "synth/FxSendEQ.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xaudio2/xaudio2.h"

class FxSendEQ360 : public FxSendEQ, public FxSend360 {
public:
    FxSendEQ360();
    virtual ~FxSendEQ360();
    OBJ_CLASSNAME(FxSendEQ360)
    OBJ_SET_TYPE(FxSendEQ360)
    virtual void Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }
    virtual void UpdateMix() { UpdateVolumes(); }
    virtual void OnParametersChanged() { FxSend360::SyncEffectParams(); }
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendEQ360)

protected:
    virtual IUnknown *CreateFx();
};
