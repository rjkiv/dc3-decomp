#pragma once
#include "xdk/XAUDIO2.h"

// size 0x14
class __declspec(uuid("24BE678A-C537-4C1C-A82F-164CFB06E7A6")) DelayEffect {
public:
    struct Params {
        Params() : bypass(false) {}
        /** "Bypass the effect and stop it from processing" */
        bool bypass; // 0x0
        float delayTime; // 0x4
        float gain; // 0x8
        /** "Depth of ping pong effect (percent)". Ranges from 0 to 100. */
        float pingPongPct; // 0xc
    };

    DelayEffect(IXAudioBatchAllocator *);
    ~DelayEffect();
    void Reset();
    void Process(float *, int, int);
    void SetParameter(int, float);
    void SetParameters(const DelayEffect::Params &);

private:
    int unk0;
    int unk4;
    float unk8;
    float unkc;
    float *unk10;
};
