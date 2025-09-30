#include "synth_xbox/FftIpp.h"
#include "utl/MemMgr.h"

void FftIpp::FftRealCcs(unsigned int *, volatile float &, unsigned int *, float &) {}

void FftIpp::FftReal(
    unsigned int *param1, volatile float &param2, unsigned int *, float &, volatile float &
) {}

FftIpp::~FftIpp() {
    if (unk8 != 0) {
        MemFree(&unk8);
    }

    if (unk14 != 0) {
        MemFree(&unk14);
    }

    if (unk20 != 0) {
        MemFree(&unk20);
    }

    if (unk2c != 0) {
        MemFree(&unk2c);
    }

    if (unk38 != 0) {
        MemFree(&unk38);
    }
}

FftIpp::FftIpp()
    : unk0(0), unk4(0), unk8(0), unkc(0), unk10(0), unk14(0), unk18(0), unk1c(0),
      unk20(0), unk24(0), unk28(0), unk2c(0), unk30(0), unk34(0), unk38(0), unk3c(0),
      unk40(0) {}

void FftIpp::SetMode(int) {}
