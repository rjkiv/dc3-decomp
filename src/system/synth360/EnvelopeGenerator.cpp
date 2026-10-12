#include "synth360/EnvelopeGenerator.h"

EnvelopeGenerator::EnvelopeGenerator() : mEnvState(0) {
    EnvelopeGeneratorParams p;
    p.AttackRate = 0;
    p.ReleaseRate = 0;
    p.StartRelease = 0;
    p.IsDone = 0;
    mAttackSamples = 0;
    mReleaseSamples = 0;
    mState = kAttack;
    SetParameters(&p, sizeof(EnvelopeGeneratorParams));
}

void EnvelopeGenerator::OnSetParameters(const EnvelopeGeneratorParams &p) {
    mAttackSamples = p.AttackRate * 48000;
    mReleaseSamples = p.ReleaseRate * 48000;
    if (p.StartRelease > 0.5f && mState != kDone) {
        mState = kRelease;
    }
}
