#pragma once
#include "xdk/XAUDIO2.h"

DEFINE_CLSID(EQEffect, 0E0F3600, B28E, 4434, 81, 0D, 21, B8, BE, 74, 06, 19);

// size 0x34C
class EQEffect {
public:
    // size 0x38
    struct Params {
        Params() : bypass(false) {}
        /** "Bypass the effect and stop it from processing" */
        bool bypass; // 0x0
        /** "High frequency cutoff, in Hz". Ranges from 0 to 24000. */
        float highFreqCutoff; // 0x4
        /** "High frequency gain, in dB". Ranges from -42 to 42. */
        float highFreqGain; // 0x8
        /** "Mid frequency cutoff, in Hz". Ranges from 0 to 24000. */
        float midFreqCutoff; // 0xc
        /** "Mid frequency bandwidth, in Hz". Ranges from 0 to 24000. */
        float midFreqBandwidth; // 0x10
        /** "Mid frequency gain, in dB". Ranges from -42 to 42. */
        float midFreqGain; // 0x14
        /** "Low frequency cutoff, in Hz". Ranges from 0 to 24000. */
        float lowFreqCutoff; // 0x18
        /** "Low frequency gain, in dB". Ranges from -42 to 42. */
        float lowFreqGain; // 0x1c
        /** "Low pass filter cutoff, in Hz". Ranges from 20 to 20000. */
        float lowPassCutoff; // 0x20
        /** "Low pass filter resonance, in dB". Ranges from -25 to 25. */
        float lowPassReso; // 0x24
        /** "High pass filter cutoff, in Hz". Ranges from 20 to 20000. */
        float highPassCutoff; // 0x28
        /** "High pass filter resonance, in dB". Ranges from -25 to 25. */
        float highPassReso; // 0x2c
        /** "Enable or disable Linkwitz-Riley mode" */
        float lrMode; // 0x30
        /** "Transition time for gain changes, in ms". Ranges from 25 to 5000. */
        float transitionTime; // 0x34
    };

    EQEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameter(int, float);
    void SetParameters(const EQEffect::Params &);

private:
    float mHighFreqCutoff; // 0x0
    float mHighFreqGain; // 0x4
    float mMidFreqCutoff; // 0x8
    float mMidFreqBandwidth; // 0xC
    float mMidFreqGain; // 0x10
    float mLowFreqCutoff; // 0x14
    float mLowFreqGain; // 0x18
    float mLPFCutoff; // 0x1C
    float mLPFReso; // 0x20
    float mHPFCutoff; // 0x24
    float mHPFReso; // 0x28
    float mLRMode; // 0x2C
    float mTransitionTime; // 0x30
    float mTTk; // 0x34
    bool mComputeHF; // 0x38
    float mHF_k; // 0x3C
    float mHF_v0; // 0x40
    float mHF_v0TT; // 0x44
    float mHF_h02; // 0x48
    float mHF_h02TT; // 0x4C
    float mHF_a; // 0x50
    bool mComputeMF; // 0x54
    float mMF_a; // 0x58
    float mMF_k; // 0x5C
    float mMF_v0; // 0x60
    float mMF_v0TT; // 0x64
    float mMF_h02; // 0x68
    float mMF_h02TT; // 0x6C
    float mMF_d; // 0x70
    bool mComputeLF; // 0x74
    float mLF_k; // 0x78
    float mLF_v0; // 0x7C
    float mLF_v0TT; // 0x80
    float mLF_h02; // 0x84
    float mLF_h02TT; // 0x88
    float mLF_a; // 0x8C
    bool mComputeLPF; // 0x90
    float mLPF_A0; // 0x94
    float mLPF_A1; // 0x98
    float mLPF_A2; // 0x9C
    float mLPF_B1; // 0xA0
    float mLPF_B2; // 0xA4
    bool mComputeHPF; // 0xA8
    float mHPF_A0; // 0xAC
    float mHPF_A1; // 0xB0
    float mHPF_A2; // 0xB4
    float mHPF_B1; // 0xB8
    float mHPF_B2; // 0xBC
    float mMF_xn1[2]; // 0xC0
    float mMF_xn2[2]; // 0xC8
    float mMF_y1n1[2]; // 0xD0
    float mMF_y1n2[2]; // 0xD8
    float mHFDelay[2]; // 0xE0
    float mLFDelay[2]; // 0xE8
    float mLPFDelayX[2][2]; // 0xF0
    float mLPFDelayY[2][2]; // 0x100
    float mHPFDelayX[2][2]; // 0x110
    float mHPFDelayY[2][2]; // 0x120
    float mLPGain; // 0x130
    float mBPGain; // 0x134
    float mHPGain; // 0x138
    float mLRB[3][4]; // 0x13C
    float mLRxv[2][3][2][5]; // 0x16C
    float mLRyv[2][3][2][5]; // 0x25C
};
