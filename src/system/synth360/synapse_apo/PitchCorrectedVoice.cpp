#include "synth360/synapse_apo/PitchCorrectedVoice.h"
#include "synth360/synapse_apo/Util.h"

DSP::Synapse::PitchCorrectedVoice::PitchCorrectedVoice()
    : unk0(0), unk4(0), unk8(0), unkc(0), unk10(0), mReleaseSmoothing(0),
      mTransposition(0), mAmount(1), mProximityEffect(0), mProximityFocus(0.5f), unk28(0),
      unk2c(0), unk30(0), unk34(0) {}

DSP::Synapse::PitchCorrectedVoice::~PitchCorrectedVoice() {}

void DSP::Synapse::PitchCorrectedVoice::SetAmount(float amt) { mAmount = amt; }
void DSP::Synapse::PitchCorrectedVoice::SetProximityEffect(float eff) {
    mProximityEffect = eff;
}
void DSP::Synapse::PitchCorrectedVoice::SetProximityFocus(float focus) {
    mProximityFocus = focus;
}
void DSP::Synapse::PitchCorrectedVoice::SetReleaseSmoothing(float r) {
    mReleaseSmoothing = r;
}

void DSP::Synapse::PitchCorrectedVoice::SetTransposition(float trans) {
    mTransposition = Util::Log(2.0f) * trans * 0.083333336f;
}

void DSP::Synapse::PitchCorrectedVoice::SetAttackSmoothing(float r) {
    unkc = r;
    unk10 = r;
}
