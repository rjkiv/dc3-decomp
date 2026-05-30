#include "dsp/BitCrushEffect.h"
#include "os/Debug.h"

BitCrushEffect::BitCrushEffect(IXAudioBatchAllocator *)
    : unk0(0), unk4(0), unk8(0), unkc(0) {}

void BitCrushEffect::SetParameters(const BitCrushEffect::Params &params) {
    unk0 = params.amount;
}

void BitCrushEffect::Process(float *fptr, int i1, int numChans) {
    MILO_ASSERT(numChans <= 2, 0x1e);
    float *fcur = fptr;
    float *fitr = fptr + 1;
    for (int i = 0; i < i1; i++, fcur += numChans, fitr += 2) {
        if (unk4 > 0) {
            *fcur = unk8;
            if (numChans == 2) {
                *fitr = unkc;
            }
            unk4--;
        } else {
            unk4 = (int)unk0;
            unk8 = *fcur;
            if (numChans == 2) {
                unkc = *fitr;
            }
        }
    }
}

void BitCrushEffect::Reset() {}
