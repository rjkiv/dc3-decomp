#include "synth_xbox/EnvelopeGenerator.h"

EnvelopeGenerator::EnvelopeGenerator() : unk8c(0) {
    EnvelopeGeneratorParams p;
    p.unk0 = 0;
    p.unk4 = 0;
    p.unk8 = 0;
    p.unkc = 0;
    unk84 = 0;
    unk88 = 0;
    unk90 = 0;
    SetParameters(&p, sizeof(EnvelopeGeneratorParams));
}

void EnvelopeGenerator::OnSetParameters(const EnvelopeGeneratorParams &p) {
    unk84 = p.unk0 * 48000;
    unk88 = p.unk4 * 48000;
    if (p.unk8 > 0.5f && unk90 != 3) {
        unk90 = 2;
    }
}
