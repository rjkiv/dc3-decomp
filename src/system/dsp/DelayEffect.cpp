#include "dsp/DelayEffect.h"
#include "Common_Xbox.h"
#include "math/Decibels.h"
#include "math/Utl.h"
#include "os/Debug.h"

#define kMaxDelaySamps 96000

DelayEffect::DelayEffect(IXAudioBatchAllocator *ix)
    : delaytime(24000), saved_writepos(0), gain(0.3f), pongpct(0.5f) {
    DspAllocate(delaybuf, 192000, ix);
}

DelayEffect::~DelayEffect() { DspFree(delaybuf); }

void DelayEffect::Reset() { DspClearBuffer(delaybuf, 192000); }

void DelayEffect::Process(float *samples, int sampct, int numChans) {
    MILO_ASSERT(numChans <= 2, 39);
    int writePos = saved_writepos;
    if (numChans == 1) {
        if (sampct <= 0)
            goto exit;
        for (int i = sampct; i != 0; i--) {
            int readPos = writePos - delaytime;
            MILO_ASSERT_RANGE(readPos, 0, kMaxDelaySamps, 50);
            readPos += kMaxDelaySamps;
            MILO_ASSERT_RANGE(writePos, 0, kMaxDelaySamps, 51);
            float this_delay = samples[readPos];
            writePos++;
            this_delay *= delaybuf[readPos];
            samples[writePos] = this_delay;
            delaybuf[writePos] = this_delay + samples[readPos];
            samples++;
        }
    } else {
        if (sampct <= 0)
            goto exit;
        float c = 1.0f - pongpct;
        float *samples_l = samples;
        float *samples_r = samples + 1;
        for (int i = sampct; i != 0; i--) {
            int readPos = writePos - delaytime;
            if (readPos < 0) {
                readPos += kMaxDelaySamps;
            }
            int rp2s = readPos + kMaxDelaySamps;
            float f11 = samples_l[0];
            float f10 = samples_l[1];
            f11 *= c;

            f11 = gain;
            f11 *= delaybuf[rp2s];
            f11 = f11 * c + ((delaybuf[readPos] * gain) * pongpct);
            samples_l[1] = f11;
            delaybuf[writePos + kMaxDelaySamps] = f10 * c + f11;
            samples_l += numChans;
        }
    }
exit:
    saved_writepos = writePos;
}

void DelayEffect::SetParameter(int idx, float val) {
    switch (idx) {
    case 0: {
        int j = val * 48000;
        delaytime = j;
        delaytime = Clamp(1, 96000 - 1, j);
    } break;
    case 1: {
        gain = DbToRatio(val);
    } break;
    case 2: {
        pongpct = val / 100;
    } break;
    default: {
        MILO_FAIL("bad parameter %i\n", idx);
    } break;
    }
}

void DelayEffect::SetParameters(const DelayEffect::Params &params) {
    SetParameter(0, params.delayTime);
    SetParameter(1, params.gain);
    SetParameter(2, params.pingPongPct);
}
