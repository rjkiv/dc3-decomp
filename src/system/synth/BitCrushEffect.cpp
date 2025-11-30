#include "synth/BitCrushEffect.h"
#include "os/Debug.h"
#include "xdk/xaudio2/xaudio2.h"

BitCrushEffect::BitCrushEffect(IXAudioBatchAllocator *)
    : unk0(0), unk4(0), unk8(0), unkc(0) {}

void BitCrushEffect::SetParameters(BitCrushEffect::Params const &params) {
    unk0 = params.unk4;
}

void BitCrushEffect::Process(float *f, int i1, int numChans) {
    MILO_ASSERT(numChans <= 2, 0x1e);
}
