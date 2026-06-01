#pragma once
#include "xdk/XAUDIO2.h"

DEFINE_CLSID(CompressionEffect, 443A5BB5, 2BD8, 45FE, AC, E8, 3B, 51, 2D, 6C, BE, 68);

// size 0x40
class CompressionEffect {
public:
    struct Params {
        Params() : bypass(false) {}
        /** "Bypass the effect and stop it from processing" */
        bool bypass; // 0x0
        /** "threshold (in dB) at which compression is applied". Ranges from -96 to 0 */
        float thresholdDB; // 0x4
        /** "Compression factor - ratio of input level to output level".
         *  Ranges from 1 to 80 */
        float ratio; // 0x8
        /** "output level for a maxed signal, in dB". Ranges from -10 to 10. */
        float outputLevel; // 0xc
        /** "Attack time in seconds". Ranges from 1.0e-3 to 1. */
        float attack; // 0x10
        /** "Release time in seconds". Ranges from 1.0e-3 to 2. */
        float release; // 0x14
        /** "Expansion factor - ratio of input level to output level.  The expander uses
         * the same threshold as the compressor.". Ranges from 1 to 20. */
        float expRatio; // 0x18
        /** "Attack time in seconds". Ranges from 1.0e-3 to 2. */
        float expAttack; // 0x1c
        /** "Release time in seconds". Ranges from 1.0e-3 to 1. */
        float expRelease; // 0x20
        /** "threshold (in dB) at which gating is applied". Ranges from -96 to 0 */
        float gateThresholdDB; // 0x24
    };

    CompressionEffect(IXAudioBatchAllocator *);
    void Reset();
    void Process(float *, int, int);
    void SetParameters(const CompressionEffect::Params &);

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
    float unk34;
    float unk38;
    float unk3c;
};
