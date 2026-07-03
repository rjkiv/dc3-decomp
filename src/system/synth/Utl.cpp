#include "Utl.h"
#include "os/FileCache.h"

class WavFileCacheHelper : public FileCacheHelper {
public:
    virtual const char *CacheFile(const char *);
};

static WavFileCacheHelper gWavFileCacheHelper;

float sMeasuresMs[7] = { 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f };

void SynthUtlInit() { FileCache::RegisterWavCacheHelper(&gWavFileCacheHelper); }

void SynthUtlTerm() {}

float CalcSpeedFromTranspose(float f1) { return std::pow(2.0, f1 * 0.083333333f); }

float CalcTransposeFromSpeed(float f1) {
    float log = std::log(f1);
    return log * 17.31234f;
}

float CalcRateForTempoSync(Symbol sym, float f1) {
    static Symbol measures[7] = { "sixteenth",      "eighth", "dotted_eighth", "quarter",
                                  "dotted_quarter", "half",   "whole" };

    float res = f1 * 0.016666668f;
    float temp = 1.0f;
    for (int i = 0; i < DIM(sMeasuresMs); i++) {
        if (measures[i] == sym) {
            temp = sMeasuresMs[i];
            break;
        }
    }
    return res / temp;
}
