#pragma once
#include "xdk/XAUDIO2.h"

DEFINE_CLSID(FlangerEffect, 443A5BB5, 2BD8, 45FE, AC, E8, 3B, 51, 2D, 6C, BE, 68);

// size 0x34
class FlangerEffect {
public:
    struct Params {
        Params() : bypass(false) {}
        /** "Bypass the effect and stop it from processing" */
        bool bypass; // 0x0
        /** "Maximum delay time in milliseconds". Ranges from 0 to 10. */
        float delayMs; // 0x4
        /** "Rate at which delay is modulated (Hz)". Ranges from 0 to 10. */
        float rate; // 0x8
        /** "Percent depth of delay modulation". Ranges from 0 to 100. */
        float depthPct; // 0xc
        /** "Percent of output that is fed back to input". Ranges from 0 to 100. */
        float feedbackPct; // 0x10
        /** "LFO phase offset between channels (for wider stereo effect)".
         * Ranges from 0 to 100. */
        float offsetPct; // 0x14
    };

    ~FlangerEffect();
    FlangerEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameters(const FlangerEffect::Params &);

private:
    float *unk0[4];
    int unk10;
    int unk14;
    float unk18;
    float unk1c;
    float unk20;
    float unk24;
    float unk28;
    float unk2c;
    float unk30;
};
