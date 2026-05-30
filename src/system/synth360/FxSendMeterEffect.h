#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "synth/FxSendMeterEffect.h"
#include "synth360/MeterEffect.h"
#include "xdk/xapilibi/xbase.h"

class FxSendMeterEffect360 : public FxSendMeterEffect, public FxSend360 {
public:
    FxSendMeterEffect360();
    virtual ~FxSendMeterEffect360();
    OBJ_CLASSNAME(FxSendMeterEffect360)
    OBJ_SET_TYPE(FxSendMeterEffect360)
    virtual void Recreate(std::vector<FxSend *> &);
    virtual void UpdateMix();
    virtual void OnParametersChanged();
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendMeterEffect360)

protected:
    virtual IUnknown *CreateFx();
    virtual void InitParams(IXAudio2SubmixVoice *, int);

    MeterEffectParams *mParams; // 0xb0
};
