#pragma once
#include "xdk/XAUDIO2.h"

DEFINE_CLSID(DistortionEffect, A46688F1, A161, 452F, AF, 1C, 3E, 63, 80, 45, 6B, DA);

// size 0x4
class DistortionEffect {
public:
    struct Params {
        Params() : bypass(false) {}
        /** "Bypass the effect and stop it from processing" */
        bool bypass; // 0x0
        /** "amount of drive". Ranges from 0 to 100. */
        float drive; // 0x4
    };

    DistortionEffect(IXAudioBatchAllocator *);
    void Process(float *, int, int);
    void SetParameters(const DistortionEffect::Params &);
    void Reset();

private:
    float unk0;
};
