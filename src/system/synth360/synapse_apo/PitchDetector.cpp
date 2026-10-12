#include "synth360/synapse_apo/PitchDetector.h"

DSP::Synapse::PitchDetector::PitchDetector(
    const std::vector<float> &vec, unsigned int ui2, unsigned int ui3
)
    : m_delayLine(vec), m_sMinPitchLag(ui2), m_sMaxPitchLag(ui2), m_fPitchLag(0),
      m_fTonality(0), m_fEnergy(0) {
    m_spectralAnalysis.SetMode((float)m_sMaxPitchLag * 1.8f, m_sMaxPitchLag);
    m_fTemp.resize(m_spectralAnalysis.GetUnk0());
    m_fWin.resize(m_spectralAnalysis.GetUnk0());
    m_fWin2.resize(m_sMaxPitchLag + 1);
}

DSP::Synapse::PitchDetector::~PitchDetector() {}
