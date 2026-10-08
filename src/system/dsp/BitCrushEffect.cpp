#include "dsp/BitCrushEffect.h"
#include "os/Debug.h"

BitCrushEffect::BitCrushEffect(IXAudioBatchAllocator *)
    : mAmount(0), mSamplesLeftToHold(0), mHoldSample1(0), mHoldSample2(0) {}

void BitCrushEffect::SetParameters(const BitCrushEffect::Params &params) {
    mAmount = params.amount;
}

void BitCrushEffect::Process(float *fptr, int i1, int numChans) {
    MILO_ASSERT(numChans <= 2, 0x1e);
    float *fcur = fptr;
    float *fitr = fptr + 1;
    for (int i = 0; i < i1; i++, fcur += numChans, fitr += 2) {
        if (mSamplesLeftToHold > 0) {
            *fcur = mHoldSample1;
            if (numChans == 2) {
                *fitr = mHoldSample2;
            }
            mSamplesLeftToHold--;
        } else {
            mSamplesLeftToHold = (int)mAmount;
            mHoldSample1 = *fcur;
            if (numChans == 2) {
                mHoldSample2 = *fitr;
            }
        }
    }
}

void BitCrushEffect::Reset() {}
