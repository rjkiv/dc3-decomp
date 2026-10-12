#include "FxSendMeterEffect.h"
#include "FxSend.h"
#include "macros.h"
#include "synth/FxSend.h"
#include "os/Debug.h"
#include "synth360/MeterEffect.h"
#include "xdk/unknwn.h"

FxSendMeterEffect360::FxSendMeterEffect360() : FxSend360(this), mMeterParams(0) {}

FxSendMeterEffect360::~FxSendMeterEffect360() { RELEASE(mMeterParams); }

void FxSendMeterEffect360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    MeterEffectParams p;
    if (mMeterParams) {
        p.pLevelData = mMeterParams->pLevelData;
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
    RELEASE(mMeterParams);
    mMeterParams = new MeterEffectParams;
    mMeterParams->pLevelData = &mChannels[0];
    voice->SetEffectParameters(0, mMeterParams, sizeof(MeterEffectParams), 0);
}

IUnknown *FxSendMeterEffect360::CreateFx() {
    return static_cast<CXAPOBase *>(new MeterEffect());
}
