#include "synth360/synapse_apo/PitchCorrectedVoice.h"
#include "synth360/synapse_apo/Util.h"

DSP::Synapse::PitchCorrectedVoice::PitchCorrectedVoice()
    : m_fDetectedNoteHz(0), m_fTargetNoteHz(0), m_fSmoothedCorrection(0),
      m_fBaseSmoothing(0), m_fAttackSmoothing(0), m_fReleaseSmoothing(0),
      m_fTransposition(0), m_fAmount(1), m_fProximityEffect(0), m_fProximityFocus(0.5f),
      m_fTonality(0), m_fEnergy(0), m_fPrevEnergy(0), m_fErrorMagnitude(0) {}

DSP::Synapse::PitchCorrectedVoice::~PitchCorrectedVoice() {}

void DSP::Synapse::PitchCorrectedVoice::SetAmount(float amt) { m_fAmount = amt; }
void DSP::Synapse::PitchCorrectedVoice::SetProximityEffect(float eff) {
    m_fProximityEffect = eff;
}
void DSP::Synapse::PitchCorrectedVoice::SetProximityFocus(float focus) {
    m_fProximityFocus = focus;
}
void DSP::Synapse::PitchCorrectedVoice::SetReleaseSmoothing(float r) {
    m_fReleaseSmoothing = r;
}

void DSP::Synapse::PitchCorrectedVoice::SetTransposition(float trans) {
    m_fTransposition = Util::Log(2.0f) * trans * 0.083333336f;
}

void DSP::Synapse::PitchCorrectedVoice::SetAttackSmoothing(float r) {
    m_fBaseSmoothing = r;
    m_fAttackSmoothing = r;
}
