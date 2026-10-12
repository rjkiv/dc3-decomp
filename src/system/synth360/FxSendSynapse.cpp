#include "synth360/FxSendSynapse.h"
#include "synth360/synapse_apo/SynapseAPO.h"
#include "xdk/XAUDIO2.h"

void FxSendSynapse360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    DSP::SynapseAPOParams params;
    params.mNoteProps[0].fAmount = mAmount;
    params.mNoteProps[0].fProximityEffect = mProximityEffect;
    params.mNoteProps[0].fProximityFocus = mProximityFocus;
    params.mNoteProps[0].bEnabled = true;
    params.mNoteProps[2].fGain = 1;
    params.mNoteProps[1].bEnabled = true;
    params.mNoteProps[0].fNoteHz = mNote1Hz;
    params.fAttackSmoothing = mAttackSmoothing;
    params.fReleaseSmoothing = mReleaseSmoothing;
    params.mNoteProps[0].fGain = 1;
    if (mNote2Hz == 0) {
        params.mNoteProps[1].fGain = mUnisonTrio ? params.mNoteProps[2].fGain : 0;
        params.mNoteProps[1].fNoteHz = params.mNoteProps[0].fNoteHz * 0.9904912f;
        params.mNoteProps[1].fProximityEffect = params.mNoteProps[0].fProximityEffect;
    } else {
        params.mNoteProps[1].fGain = params.mNoteProps[2].fGain;
        params.mNoteProps[1].fNoteHz = mNote2Hz * 0.99601597f;
        params.mNoteProps[1].fProximityEffect = 0;
    }
    params.mNoteProps[2].bEnabled = true;
    params.mNoteProps[2].fAmount = mAmount;
    params.mNoteProps[2].fProximityFocus = mProximityFocus;
    if (mNote3Hz == 0) {
        if (!mUnisonTrio) {
            params.mNoteProps[2].fGain = 0;
        }
        params.mNoteProps[2].fNoteHz = params.mNoteProps[0].fNoteHz * 1.0096f;
        params.mNoteProps[2].fProximityEffect = params.mNoteProps[0].fProximityEffect;
    } else {
        params.mNoteProps[2].fNoteHz = mNote3Hz * 1.004f;
        params.mNoteProps[2].fProximityEffect = 0;
    }
    params.mNoteProps[1].fAmount = params.mNoteProps[0].fAmount;
    params.mNoteProps[1].fProximityFocus = params.mNoteProps[0].fProximityFocus;
    voice->SetEffectParameters(0, &params, sizeof(params), 0);
}

IUnknown *FxSendSynapse360::CreateFx() {
    return static_cast<CXAPOBase *>(new DSP::SynapseAPO());
}
