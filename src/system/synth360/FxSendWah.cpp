#include "FxSendWah.h"
#include "FxSend.h"
#include "dsp/WahEffect.h"
#include "dsp/StandardEffect.h"
#include "math/Utl.h"
#include "synth/Utl.h"

FxSendWah360::FxSendWah360() : FxSend360(this) {}

void FxSendWah360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    WahEffect::Params p;
    p.resonance = mResonance;
    p.upperFreq = mUpperFreq;
    p.lowerFreq = mLowerFreq;
    p.magic = mMagic;
    if (mTempoSync) {
        p.lfoFreq = CalcRateForTempoSync(mSyncType, mTempo);
    } else {
        p.lfoFreq = mLfoFreq;
    }
    if (mTempoSync) {
        p.beatFrac = Clamp(0.0f, 1.0f, mBeatFrac);
    } else {
        p.beatFrac = -1;
    }
    p.distAmount = mDistAmount;
    p.bypass = mBypass;
    p.frequency = mFrequency;
    p.autoWah = mAutoWah != false;
    voice->SetEffectParameters(0, &p, sizeof(p), 0);
}

IUnknown *FxSendWah360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<WahEffect>());
}
