#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendDistortion.h"
#include "xdk/xapilibi/xbase.h"

class FxSendDistortion360 : public FxSendDistortion, public FxSend360 {
public:
    FxSendDistortion360();
    virtual ~FxSendDistortion360();
    OBJ_CLASSNAME(FxSendDistortion360)
    OBJ_SET_TYPE(FxSendDistortion360)
    virtual void Recreate(std::vector<FxSend *> &);
    virtual void UpdateMix();
    virtual void OnParametersChanged();

    NEW_OBJ(FxSendDistortion360)

    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

protected:
    virtual IUnknown *CreateFx();
};
