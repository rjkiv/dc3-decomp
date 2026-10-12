#include "synth360/synapse_apo/GranularSynth.h"

DSP::Synapse::GranularSynth::GranularSynth(
    const std::vector<float> &vec, unsigned int ui2, unsigned int ui3, unsigned int ui4
)
    : m_delayLine(vec), m_fPitchLag(ui3), m_fNewGranuleStart(0), m_fTonality(0),
      m_sMinPitchLag(ui3), m_sMaxPitchLag(ui4), m_iCurOut(0), m_iBufferOffset(0),
      m_granules((ui4 << 1) / (ui3 + 0x10) * ui2), m_voices(ui2) {
    m_fCrossfades.resize(4);
}

DSP::Synapse::GranularSynth::~GranularSynth() {}
