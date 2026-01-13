#include "Utl.h"

// No idea if these numbers are right - need to align with measures syms below
float measuresMs[7] = {0.0625, 0.125, 0.1875, 0.25, 0.375, 0.5, 1.0};

void SynthUtlInit() {
    FileCache::RegisterWavCacheHelper(gWavFileCacheHelper);
}

float CalcSpeedFromTranspose(float f1) {
    return std::pow(2.0, f1 * 0.083333333f);
}

float CalcTransposeFromSpeed(float f1) {
    float log = std::log(f1);
    return log * 17.31234f;
}

float CalcRateForTempoSync(Symbol sym, float f1) {
    static Symbol measures[7] = {"sixteenth", "eighth", "dotted_eighth",
        "quarter", "dotted_quarter", "half", "whole"};

    float temp = 1.0;

    for (int i = 0; i < 7; i++) {
        if (sym == measures[i]) {
            temp = measuresMs[i];
            break;
        }
    }
    return (f1 * 0.016666667f) / temp;
}