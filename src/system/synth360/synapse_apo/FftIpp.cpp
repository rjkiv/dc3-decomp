#include "synth360/synapse_apo/FftIpp.h"
#include "os/Debug.h"
#include "synth360/synapse_apo/FFT.h"
#include "utl/MemMgr.h"
#include <cstring>

FftIpp::FftIpp() : unk0(0), unk4(0) {}

FftIpp::~FftIpp() {}

void FftIpp::SetMode(int i1) {
    unk0 = i1;
    unk4 = 1;
    for (; 1 << unk4 < (int)unk0; unk4++)
        ;
    unk8.resize(i1);
    unk14.resize(unk0);
    unk20.resize(unk0);
    unk38.resize(unk0);
    CalculateSinCosTable((int)unk0 / 2, &unk38[0]);
}

void FftIpp::FftRealCcs(float const *__restrict f1, float *__restrict f2) {
    if (unk0) {
        memcpy(&unk20[0], f1, unk0 * 4);
    }
    int iRetVal = FFTRealForward(&unk20[0], unk0, &unk38[0]);
    MILO_ASSERT(iRetVal == 0, 0x65);
    if (unk0) {
        memcpy(f2, &unk20[0], unk0 * 4);
    }
    f2[unk0] = f2[1];
    f2[unk0 + 1] = 0;
    f2[1] = 0;
}

void FftIpp::FftReal(
    float const *__restrict f1, float *__restrict f2, float *__restrict f3
) {
    if (unk0) {
        memcpy(&unk20[0], f1, unk0 << 2);
    }
    FFTRealForward(&unk20[0], unk0, &unk38[0]);
    for (int i = 1; i < unk0 >> 1; i++) {
        f2[i] = unk20[i];
        f3[i] = unk20[i];
    }
    f3[0] = 0;
    f2[0] = unk20[0];
    f2[unk0] = unk20[1];
    f3[unk0] = 0;
}
