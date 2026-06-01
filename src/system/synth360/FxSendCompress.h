#pragma once
#include "FxSend.h"
#include "obj/Object.h"
#include "synth/FxSendCompress.h"
#include "xdk/xaudio2/xaudio2.h"

class FxSendCompress360 : public FxSendCompress, public FxSend360 {
public:
    FxSendCompress360();
    virtual ~FxSendCompress360();
    OBJ_CLASSNAME(FxSendCompress360)
    OBJ_SET_TYPE(FxSendCompress360)
    virtual void Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }
    virtual void UpdateMix() { UpdateVolumes(); }
    virtual void OnParametersChanged() { FxSend360::SyncEffectParams(); }
    virtual void SyncEffectParams(IXAudio2SubmixVoice *) const;

    NEW_OBJ(FxSendCompress360)

protected:
    virtual IUnknown *CreateFx();
};
