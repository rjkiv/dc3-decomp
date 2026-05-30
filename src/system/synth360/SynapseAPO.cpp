#include "synth360/SynapseAPO.h"
#include "synth360/Synapse_dsp.h"

DSP::SynapseAPO::SynapseAPO() : mSynapse(0) { SetSamplingRate(48000); }
DSP::SynapseAPO::~SynapseAPO() { delete mSynapse; }

void DSP::SynapseAPO::OnSetParameters(const SynapseAPOParams &params) {
    for (unsigned int i = 0; i < 3; i++) {
        if (mParams.mNoteProps[i].enabled != params.mNoteProps[i].enabled) {
            mSynapse->SetVoiceEnabled(i, params.mNoteProps[i].enabled);
        }
        if (mParams.mNoteProps[i].gain != params.mNoteProps[i].gain) {
            mSynapse->SetVoiceGain(i, params.mNoteProps[i].gain);
        }
        if (mParams.mNoteProps[i].targetNote != params.mNoteProps[i].targetNote) {
            mSynapse->SetVoiceTargetNote(i, params.mNoteProps[i].targetNote);
        }
        if (mParams.mNoteProps[i].transposition != params.mNoteProps[i].transposition) {
            mSynapse->SetVoiceTransposition(i, params.mNoteProps[i].transposition);
        }
        if (mParams.mNoteProps[i].amount != params.mNoteProps[i].amount) {
            mSynapse->SetVoiceAmount(i, params.mNoteProps[i].amount);
        }
        if (mParams.mNoteProps[i].proximityEffect
            != params.mNoteProps[i].proximityEffect) {
            mSynapse->SetVoiceProximityEffect(i, params.mNoteProps[i].proximityEffect);
        }
        if (mParams.mNoteProps[i].proximityFocus != params.mNoteProps[i].proximityFocus) {
            mSynapse->SetVoiceProximityFocus(i, params.mNoteProps[i].proximityFocus);
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
