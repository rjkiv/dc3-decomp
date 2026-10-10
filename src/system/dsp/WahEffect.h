#pragma once
#include "xdk/XAUDIO2.h"

DEFINE_CLSID(WahEffect, 20F3EB49, FF6B, 41A0, 8A, 09, 35, 31, A3, 50, 1A, E1);

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
    float mResonance; // 0x0
    float mMinFreq; // 0x4
    float mMaxFreq; // 0x8
    float mLfoFreq; // 0xC
    float mMagic; // 0x10
    float mBeatFrac; // 0x14
    float mDistAmount; // 0x18
    float mAutoWah; // 0x1C
    float mFrequency; // 0x20
    float mSmoothCutoff; // 0x24
    float mPreviousAutoWah; // 0x28
    int mManualWahRecoveryTimer; // 0x2C
    float mLastPhase; // 0x30
    float mA1[2]; // 0x34
    float mA2[2]; // 0x3C
    float mInSample; // 0x44
    float mOutSample; // 0x48
    float mDump; // 0x4C
};
