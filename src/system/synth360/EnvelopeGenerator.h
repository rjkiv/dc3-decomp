#pragma once
#include "utl/PoolAlloc.h"
#include "xdk/XAUDIO2.h"

// size 0x10
struct EnvelopeGeneratorParams {
    float AttackRate; // 0x0
    float ReleaseRate; // 0x4
    float StartRelease; // 0x8
    float IsDone; // 0xc

    POOL_OVERLOAD(EnvelopeGeneratorParams, 0x1E);
};

DEFINE_CLSID(EnvelopeGenerator, B4D4C8AA, A20D, 40A1, 84, A7, 64, 19, 35, 51, A9, BC);

class EnvelopeGenerator
    : public ATG::CSampleXAPOBase<EnvelopeGenerator, EnvelopeGeneratorParams> {
public:
    enum State {
        kAttack = 0x0000,
        kSustain = 0x0001,
        kRelease = 0x0002,
        kDone = 0x0003
    };

    EnvelopeGenerator();
    virtual void OnSetParameters(const EnvelopeGeneratorParams &);
    virtual void DoProcess(
        const EnvelopeGeneratorParams &, float *__restrict, unsigned int, unsigned int
    );

    POOL_OVERLOAD(EnvelopeGenerator, 0x2A);

private:
    int mAttackSamples; // 0x84
    int mReleaseSamples; // 0x88
    float mEnvState; // 0x8c
    State mState; // 0x90
};
