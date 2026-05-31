#include "synth360/synapse_apo/GranularSynth.h"

DSP::Synapse::GranularSynth::GranularSynth(
    const std::vector<float> &vec, unsigned int ui2, unsigned int ui3, unsigned int ui4
)
    : unk0(vec), unk4(ui3), unk8(0), unkc(0), unk10(ui3), unk14(ui4), unk18(0), unk1c(0),
      unk20((ui4 << 1) / (ui3 + 0x10) * ui2), unk2c(ui2) {
    unk38.resize(4);
}

DSP::Synapse::GranularSynth::~GranularSynth() {}
