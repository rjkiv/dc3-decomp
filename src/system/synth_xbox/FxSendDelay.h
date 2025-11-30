#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "synth/FxSendDelay.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xaudio2/xaudio2.h"

class FxSendDelay360 : public FxSendDelay, public FxSend360 {
public:
    virtual ~FxSendDelay360();
    OBJ_CLASSNAME(FxSendDelay360)
    OBJ_SET_TYPE(FxSendDelay360)
    virtual void Recreate(std::vector<FxSend *> &);
    virtual void UpdateMix();
    virtual void OnParametersChanged();
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendDelay360)

    FxSendDelay360();

protected:
    virtual IUnknown *CreateFx();
};
