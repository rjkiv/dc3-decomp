#pragma once
#include "xdk/XAUDIO2.h"

// size 0x50
class WahEffect {
public:
    struct Params {
        Params()
            : bypass(false), resonance(7), upperFreq(5000), lowerFreq(1000),
              lfoFreq(1.35f), magic(0.3f), beatFrac(-1), distAmount(0.5f), autoWah(true),
              frequency(0.5f) {}
        /** "Bypass the effect and stop it from processing" */
        bool bypass; // 0x0
        /** "amount of resonance (1-10)" */
        float resonance; // 0x4
        /** "high frequency peak of resonant filter (Hz)". Ranges from 100 to 10000. */
        float upperFreq; // 0x8
        /** "low frequency peak of resonant filter (Hz)". Ranges from 100 to 10000. */
        float lowerFreq; // 0xc
        /** "rate of LFO oscillations (Hz)". Ranges from 0.1 to 10. */
        float lfoFreq; // 0x10
        /** "magic number (0-1)" */
        float magic; // 0x14
        float beatFrac; // 0x18
        /** "Post wah distortion amount". Ranges from 0 to 1. */
        float distAmount; // 0x1c
        bool autoWah; // 0x20
        float frequency; // 0x24
    };

    WahEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameters(const WahEffect::Params &);

private:
    float unk0;
    float unk4;
    float unk8;
    float unkc;
    float unk10;
    float unk14;
    float unk18;
    float unk1c;
    float unk20;
    float unk24;
    float unk28;
    int unk2c;
    float unk30;
    float unk34;
    float unk38;
    float unk3c;
    float unk40;
    float unk44;
    float unk48;
    int unk4c;
};
