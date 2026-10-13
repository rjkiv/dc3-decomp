#include "synth360/synapse_apo/SynapseAPO.h"
#include "synth360/synapse_apo/Synapse_dsp.h"

DSP::SynapseAPO::SynapseAPO() : m_pSynapse(0) { SetSamplingRate(48000); }
DSP::SynapseAPO::~SynapseAPO() { delete m_pSynapse; }

void DSP::SynapseAPO::OnSetParameters(const SynapseAPOParams &params) {
    for (unsigned int i = 0; i < 3; i++) {
        const SynapseAPOVoiceParams &cur = m_params.mNoteProps[i];
        const SynapseAPOVoiceParams &next = params.mNoteProps[i];
        if (cur.bEnabled != next.bEnabled) {
            m_pSynapse->SetVoiceEnabled(i, next.bEnabled);
        }
        if (cur.fGain != next.fGain) {
            m_pSynapse->SetVoiceGain(i, next.fGain);
        }
        if (cur.fNoteHz != next.fNoteHz) {
            m_pSynapse->SetVoiceTargetNote(i, next.fNoteHz);
        }
        if (cur.fTransposition != next.fTransposition) {
            m_pSynapse->SetVoiceTransposition(i, next.fTransposition);
        }
        if (cur.fAmount != next.fAmount) {
            m_pSynapse->SetVoiceAmount(i, next.fAmount);
        }
        if (cur.fProximityEffect != next.fProximityEffect) {
            m_pSynapse->SetVoiceProximityEffect(i, next.fProximityEffect);
        }
        if (cur.fProximityFocus != next.fProximityFocus) {
            m_pSynapse->SetVoiceProximityFocus(i, next.fProximityFocus);
        }
    }
    if (m_params.fAttackSmoothing != params.fAttackSmoothing) {
        m_pSynapse->SetAttackSmoothing(params.fAttackSmoothing);
    }
    if (m_params.fReleaseSmoothing != params.fReleaseSmoothing) {
        m_pSynapse->SetReleaseSmoothing(params.fReleaseSmoothing);
    }
    m_params = params;
}

void DSP::SynapseAPO::DoProcess(
    const SynapseAPOParams &, float *__restrict f, unsigned int ui, unsigned int
) {
    if (m_pSynapse) {
        m_pSynapse->ProcessInPlace(ui, f);
    }
}

void DSP::SynapseAPO::SetSamplingRate(float rate) {
    delete m_pSynapse;
    m_pSynapse = new Synapse::Synapse(rate);
}
