#include "synth360/synapse_apo/Synapse_dsp.h"
#include "synth360/synapse_apo/Biquad.h"
#include "synth360/synapse_apo/FilterCoeffs.h"
#include "synth360/synapse_apo/GranularSynth.h"
#include "synth360/synapse_apo/PeakDetector.h"
#include "synth360/synapse_apo/PitchDetector.h"
#include <cmath>

namespace {
    float Time2IirA(float f1, float f2) {
        if (f1 > 0) {
            return 1 - expf(-1 / (f1 * f2));
        } else {
            return 1;
        }
    }
}

DSP::Synapse::Synapse::Synapse(float rate)
    : m_sPitchDetectionStride(64), m_fSamplingRate(rate) {
    float f7 = m_fSamplingRate * 0.4f < 0 ? -0.5f : 0.5f;
    float f8 = m_fSamplingRate * 0.0015384615f < 0 ? -0.5f : 0.5f;
    float fvar1 = m_fSamplingRate * 0.016666668f < 0 ? -0.5f : 0.5f;
    unsigned int u25 = f7 + m_fSamplingRate * 0.4f;
    m_sMinPitchLag = (f8 + m_fSamplingRate * 0.0015384615f);
    m_sMaxPitchLag = (fvar1 + m_fSamplingRate * 0.016666668f);
    m_fInputDelay.resize(u25);
    m_fDnInputDelay.resize(m_fInputDelay.size());
    m_iInputDelayOffset = 0;
    m_fGain = 1;
    m_pPitchDetector =
        new PitchDetector(m_fDnInputDelay, (m_sMinPitchLag + 3U) / 4, m_sMaxPitchLag / 4);
    m_fTonality = 0;
    m_fEnergy = 0;
    m_fTonalityThreshold = 0.35f;
    m_fPitchLag = m_sMinPitchLag;
    m_pPeakDetector = new PeakDetector(m_fInputDelay, m_sMinPitchLag, m_sMaxPitchLag);
    m_voices.resize(3);
    m_fTempOutputs.resize(m_voices.size());
    m_pTempOutputs.resize(m_voices.size());
    for (int i = 0; i < m_fTempOutputs.size(); i++) {
        m_fTempOutputs[i].resize(0x2000);
        m_pTempOutputs[i] = &m_fTempOutputs[i][0];
    }
    m_pGranularSynth =
        new GranularSynth(m_fInputDelay, m_voices.size(), m_sMinPitchLag, m_sMaxPitchLag);
    for (int i = 0; i < m_voices.size(); i++) {
    }
    float coeffs[8];
    LowpassCoefficients(coeffs, m_fSamplingRate, 7862.0f, 0.707f);
    m_pPreFilterLowpass = new Biquad(coeffs);
    HighpassCoefficients(coeffs, m_fSamplingRate / 4, 340.0f, 0.707f);
    m_pPreFilterHighpass = new Biquad(coeffs);
    m_fAvgSignal = 0;
    m_kAvgSignalSmoothing = Time2IirA(0.008160001f, m_fSamplingRate / 4);
    SetAttackSmoothing(30);
    SetReleaseSmoothing(80);
}

DSP::Synapse::Synapse::~Synapse() {}

void DSP::Synapse::Synapse::SetVoiceEnabled(unsigned int idx, bool enabled) {
    m_pGranularSynth->SetVoiceEnabled(idx, enabled);
}

void DSP::Synapse::Synapse::SetVoiceTargetNote(unsigned int idx, float note) {
    m_voices[idx].SetTargetNote(note);
}

void DSP::Synapse::Synapse::SetVoiceGain(unsigned int idx, float gain) {
    m_pGranularSynth->SetVoiceGain(idx, gain);
}

void DSP::Synapse::Synapse::SetVoiceTransposition(unsigned int idx, float trans) {
    m_voices[idx].SetTransposition(trans);
}

void DSP::Synapse::Synapse::SetVoiceAmount(unsigned int idx, float amt) {
    m_voices[idx].SetAmount(amt);
}

void DSP::Synapse::Synapse::SetVoiceProximityEffect(unsigned int idx, float effect) {
    m_voices[idx].SetProximityEffect(effect);
}

void DSP::Synapse::Synapse::SetVoiceProximityFocus(unsigned int idx, float focus) {
    m_voices[idx].SetProximityFocus(focus);
}

void DSP::Synapse::Synapse::SetAttackSmoothing(float a) {
    float iir = Time2IirA(a / 1000 / (float)m_sPitchDetectionStride, m_fSamplingRate);
    for (int i = 0; i < m_voices.size(); i++) {
        m_voices[i].SetAttackSmoothing(iir);
    }
}

void DSP::Synapse::Synapse::SetReleaseSmoothing(float r) {
    float iir = Time2IirA(r / 1000 / (float)m_sPitchDetectionStride, m_fSamplingRate);
    for (int i = 0; i < m_voices.size(); i++) {
        m_voices[i].SetReleaseSmoothing(iir);
    }
}
