#include "FxSendMeterEffect.h"
#include "FxSend.h"
#include "macros.h"
#include "synth/FxSend.h"
#include "os/Debug.h"
#include "synth360/MeterEffect.h"
#include "xdk/unknwn.h"

FxSendMeterEffect360::FxSendMeterEffect360() : FxSend360(this), mParams(0) {}

FxSendMeterEffect360::~FxSendMeterEffect360() { RELEASE(mParams); }

void FxSendMeterEffect360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendMeterEffect360::UpdateMix() { UpdateVolumes(); }

void FxSendMeterEffect360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

void FxSendMeterEffect360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    MeterEffectParams p;
    if (mParams) {
        p.unk0 = mParams->unk0;
    }
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

void FxSendMeterEffect360::InitParams(IXAudio2SubmixVoice *voice, int numChannels) {
    mChannels.clear();
    switch (numChannels) {
    case 1:
        mChannels.push_back("center");
        break;
    case 2: {
        LevelData left("left");
        LevelData right("right");
        mChannels.push_back(left);
        mChannels.push_back(right);
        break;
    }
    default:
        MILO_NOTIFY("InitParams only supports up to 2 channels");
        break;
    }
    // unsure about this part
    RELEASE(mParams);
    mParams = new MeterEffectParams();
    mParams->unk0 = (void *)&mChannels[0];
    voice->SetEffectParameters(0, mParams, sizeof(MeterEffectParams), 0);
}

IUnknown *FxSendMeterEffect360::CreateFx() {
    return static_cast<CXAPOBase *>(new MeterEffect());
}
