#include "synth360/synapse_apo/PeakDetector.h"

DSP::Synapse::PeakDetector::PeakDetector(
    const std::vector<float> &vec, unsigned int ui2, unsigned int ui3
)
    : unk0(vec), unk4(0), unk8(ui2), unkc(ui3), unk10(0), unk14(0), unk18(0), unk1c(0),
      unk20(0), unk24(0), unk28(0), unk2c(0), unk30(0) {}
