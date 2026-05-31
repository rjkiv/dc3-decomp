#include "synth360/synapse_apo/PitchDetector.h"

DSP::Synapse::PitchDetector::PitchDetector(
    const std::vector<float> &vec, unsigned int ui2, unsigned int ui3
)
    : unk0(vec), unk4(ui2), unk8(ui2), unkc(0), unk10(0), unk14(0) {
    unk18.SetMode((float)unk8 * 1.8f, unk8);
    unk100.resize(unk18.GetUnk0());
    unk10c.resize(unk18.GetUnk0());
    unk118.resize(unk8 + 1);
}

DSP::Synapse::PitchDetector::~PitchDetector() {}
