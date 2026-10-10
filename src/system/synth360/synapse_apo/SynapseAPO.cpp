#include "synth360/synapse_apo/SynapseAPO.h"
#include "synth360/synapse_apo/Synapse_dsp.h"

DSP::SynapseAPO::SynapseAPO() : mSynapse(0) { SetSamplingRate(48000); }
DSP::SynapseAPO::~SynapseAPO() { delete mSynapse; }

void DSP::SynapseAPO::OnSetParameters(const SynapseAPOParams &params) {
    for (unsigned int i = 0; i < 3; i++) {
        const VoiceParams &cur = mParams.mNoteProps[i];
        const VoiceParams &next = params.mNoteProps[i];
        if (cur.enabled != next.enabled) {
            mSynapse->SetVoiceEnabled(i, next.enabled);
        }
        if (cur.gain != next.gain) {
            mSynapse->SetVoiceGain(i, next.gain);
        }
        if (cur.targetNote != next.targetNote) {
            mSynapse->SetVoiceTargetNote(i, next.targetNote);
        }
        if (cur.transposition != next.transposition) {
            mSynapse->SetVoiceTransposition(i, next.transposition);
        }
        if (cur.amount != next.amount) {
            mSynapse->SetVoiceAmount(i, next.amount);
        }
        if (cur.proximityEffect != next.proximityEffect) {
            mSynapse->SetVoiceProximityEffect(i, next.proximityEffect);
        }
        if (cur.proximityFocus != next.proximityFocus) {
            mSynapse->SetVoiceProximityFocus(i, next.proximityFocus);
        }
    }
    if (mParams.attackSmoothing != params.attackSmoothing) {
        mSynapse->SetAttackSmoothing(params.attackSmoothing);
    }
    if (mParams.releaseSmoothing != params.releaseSmoothing) {
        mSynapse->SetReleaseSmoothing(params.releaseSmoothing);
    }
    mParams = params;
}

void DSP::SynapseAPO::DoProcess(
    const SynapseAPOParams &, float *__restrict f, unsigned int ui, unsigned int
) {
    if (mSynapse) {
        mSynapse->ProcessInPlace(ui, f);
    }
}

void DSP::SynapseAPO::SetSamplingRate(float rate) {
    delete mSynapse;
    mSynapse = new Synapse::Synapse(rate);
}
