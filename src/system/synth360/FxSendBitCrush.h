#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendBitCrush.h"
#include "xdk/xapilibi/xbase.h"

class FxSendBitCrush360 : public FxSendBitCrush, public FxSend360 {
public:
    FxSendBitCrush360();
    virtual ~FxSendBitCrush360();
    OBJ_CLASSNAME(FxSendBitCrush360)
    OBJ_SET_TYPE(FxSendBitCrush360)
    virtual void Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }
    virtual void UpdateMix() { UpdateVolumes(); }
    virtual void OnParametersChanged() { FxSend360::SyncEffectParams(); }

    NEW_OBJ(FxSendBitCrush360)

    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

protected:
    virtual IUnknown *CreateFx();
};
