#include "synth360/FxSendSynapse.h"
#include "synth360/synapse_apo/SynapseAPO.h"
#include "xdk/XAUDIO2.h"

void FxSendSynapse360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    DSP::SynapseAPOParams params;

    params.voices[0].fProximityEffect = mProximityEffect;
    params.voices[0].fNoteHz = mNote1Hz;
    params.voices[0].fGain = 1;
    params.voices[0].bEnabled = true;
    params.voices[1].bEnabled = true;
    params.voices[0].fAmount = mAmount;
    params.voices[1].fAmount = mAmount;
    params.voices[0].fProximityFocus = mProximityFocus;
    params.voices[1].fProximityFocus = mProximityFocus;
    params.fAttackSmoothing = mAttackSmoothing;
    params.fReleaseSmoothing = mReleaseSmoothing;

    float gain = 1;
    if (mNote2Hz == 0) {
        gain = mUnisonTrio ? 1.0f : 0.0f;
        params.voices[1].fNoteHz = mNote1Hz * 0.9904912f;
        params.voices[1].fProximityEffect = mProximityEffect;
    } else {
        params.voices[1].fNoteHz = mNote2Hz * 0.99601597f;
        params.voices[1].fProximityEffect = 0;
    }
    params.voices[1].fGain = gain;

    params.voices[2].bEnabled = true;
    params.voices[2].fAmount = mAmount;
    params.voices[2].fProximityFocus = mProximityFocus;

    gain = 1;
    if (mNote3Hz == 0) {
        gain = mUnisonTrio ? 1.0f : 0.0f;
        params.voices[2].fNoteHz = mNote1Hz * 1.0096f;
        params.voices[2].fProximityEffect = mProximityEffect;
    } else {
        params.voices[2].fNoteHz = mNote3Hz * 1.004f;
        params.voices[2].fProximityEffect = 0;
    }
    params.voices[2].fGain = gain;

    voice->SetEffectParameters(0, &params, sizeof(params), 0);
}

IUnknown *FxSendSynapse360::CreateFx() {
    return static_cast<CXAPOBase *>(new DSP::SynapseAPO());
}
