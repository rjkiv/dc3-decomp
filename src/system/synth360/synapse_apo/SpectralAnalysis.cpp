#include "synth360/synapse_apo/SpectralAnalysis.h"
#include "math/Trig.h"

void DSP::SpectralAnalysis::SetMode(unsigned int ui1, unsigned int ui2) {
    unk0 = ui1;
    unk4 = 8;
    if (ui2 == -1) {
        ui2 = ui1;
    }
    for (; unk4 < unk0 + ui2; unk4 <<= 1)
        ;
    unk8 = (unk4 >> 1) + 1;
    unkc.SetMode(unk4);
    unk50.SetMode(unk4 >> 1);
    unk94.assign(unk4, 0);
    unka0.resize(unk4 + 2);
    unkc4.resize((unk4 >> 1) + 1);
    unkd0.resize((unk4 >> 1) + 1);
    unkac.resize(unk4 >> 1);
    unkb8.resize(unk4 >> 1);

    for (int i = 0; i < unk4 >> 1; i++) {
        float x = ((float)i * PI) / (float)(unk4 >> 1);
        unkac[i] = sinf(x);
        unkb8[i] = cosf(x);
    }
}
