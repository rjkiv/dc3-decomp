#include "synth_xbox/FftIpp.h"
#include "utl/MemMgr.h"

// void FftIpp::FftRealCcs(float const *__restrict, float *__restrict) {}

// void FftIpp::FftReal(float const *__restrict, float *__restrict, float *__restrict) {}

FftIpp::FftIpp()
    : unk0(0), unk4(0), unk8(0), unkc(0), unk10(0), unk14(0), unk18(0), unk1c(0),
      unk20(0), unk24(0), unk28(0), unk2c(0), unk30(0), unk34(0), unk38(0), unk3c(0),
      unk40(0) {}

FftIpp::~FftIpp() {
    if (unk38 != 0) {
        MemFree(unk38);
    }
    if (unk2c != 0) {
        MemFree(unk2c);
    }
    if (unk20 != 0) {
        MemFree(unk20);
    }
    if (unk14 != 0) {
        MemFree(unk14);
    }
    if (unk8 != 0) {
        MemFree(unk8);
    }
}

// void FftIpp::SetMode(int) {}
