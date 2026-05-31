#pragma once
#include "xdk/XAUDIO2.h"

// size 0x34C
class __declspec(uuid("0E0F3600-B28E-4434-810D-21B8BE740619")) EQEffect {
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
    float unk2c;
    float unk30;
    u32 unk34;
    bool unk38;
    float unk3c;
    float unk40;
    float unk44;
    float unk48;
    float unk4c;
    float unk50;
    bool unk54;
    float unk58;
    float unk5c;
    float unk60;
    float unk64;
    float unk68;
    float unk6c;
    float unk70;
    bool unk74;
    float unk78;
    float unk7c;
    float unk80;
    float unk84;
    float unk88;
    float unk8c;
    bool unk90;
    float unk94;
    float unk98;
    float unk9c;
    float unka0;
    float unka4;
    bool unka8;
    float unkac;
    float unkb0;
    float unkb4;
    float unkb8;
    float unkbc;
    char buffer[0x28c];
};
