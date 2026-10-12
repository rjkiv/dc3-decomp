#include "synth360/synapse_apo/PeakDetector.h"

DSP::Synapse::PeakDetector::PeakDetector(
    const std::vector<float> &vec, unsigned int ui2, unsigned int ui3
)
    : m_delayLine(vec), m_fOutsidePitchLag(0), m_sMinPitchLag(ui2), m_sMaxPitchLag(ui3),
      m_iLastPeak(0), m_iPeakCand(0), m_iStartPeakPeriod(0), m_sPitchLag(0),
      m_bInsidePeak(0), m_fLockedPitchLag(0), m_fLastPeak(0), m_fMaxPeakCand(0),
      m_fNewlyDetectedGranule(0) {}

DSP::Synapse::PeakDetector::~PeakDetector() {}
