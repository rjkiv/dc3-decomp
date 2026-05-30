#include "synth360/FxSendSynapse.h"
#include "synth360/synapse_apo/SynapseAPO.h"
#include "xdk/XAUDIO2.h"

void FxSendSynapse360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    DSP::SynapseAPOParams params;
    params.mNoteProps[0].amount = mAmount;
    params.mNoteProps[0].proximityEffect = mProximityEffect;
    params.mNoteProps[0].proximityFocus = mProximityFocus;
    params.mNoteProps[0].enabled = true;
    params.mNoteProps[2].gain = 1;
    params.mNoteProps[1].enabled = true;
    params.mNoteProps[0].targetNote = mNote1Hz;
    params.attackSmoothing = mAttackSmoothing;
    params.releaseSmoothing = mReleaseSmoothing;
    params.mNoteProps[0].gain = 1;
    if (mNote2Hz == 0) {
        params.mNoteProps[1].gain = mUnisonTrio ? params.mNoteProps[2].gain : 0;
        params.mNoteProps[1].targetNote = params.mNoteProps[0].targetNote * 0.9904912f;
        params.mNoteProps[1].proximityEffect = params.mNoteProps[0].proximityEffect;
    } else {
        params.mNoteProps[1].gain = params.mNoteProps[2].gain;
        params.mNoteProps[1].targetNote = mNote2Hz * 0.99601597f;
        params.mNoteProps[1].proximityEffect = 0;
    }
    params.mNoteProps[2].enabled = true;
    params.mNoteProps[2].amount = mAmount;
    params.mNoteProps[2].proximityFocus = mProximityFocus;
    if (mNote3Hz == 0) {
        if (!mUnisonTrio) {
            params.mNoteProps[2].gain = 0;
        }
        params.mNoteProps[2].targetNote = params.mNoteProps[0].targetNote * 1.0096f;
        params.mNoteProps[2].proximityEffect = params.mNoteProps[0].proximityEffect;
    } else {
        params.mNoteProps[2].targetNote = mNote3Hz * 1.004f;
        params.mNoteProps[2].proximityEffect = 0;
    }
    params.mNoteProps[1].amount = params.mNoteProps[0].amount;
    params.mNoteProps[1].proximityFocus = params.mNoteProps[0].proximityFocus;
    voice->SetEffectParameters(0, &params, sizeof(params), 0);
}

IUnknown *FxSendSynapse360::CreateFx() {
    return static_cast<CXAPOBase *>(new DSP::SynapseAPO());
}
