#include "synth360/synapse_apo/Synapse_dsp.h"
#include "synth360/synapse_apo/Biquad.h"
#include "synth360/synapse_apo/FilterCoeffs.h"
#include "synth360/synapse_apo/GranularSynth.h"
#include "synth360/synapse_apo/PeakDetector.h"
#include "synth360/synapse_apo/PitchDetector.h"
#include <cmath>

namespace {
    float Time2IirA(float f1, float f2) {
        if (f1 > 0) {
            return 1 - expf(-1 / (f1 * f2));
        } else {
            return 1;
        }
    }
}

DSP::Synapse::Synapse::Synapse(float rate) : unk24(64), mSampleRate(rate) {
    float f7 = mSampleRate * 0.4f < 0 ? -0.5f : 0.5f;
    float f8 = mSampleRate * 0.0015384615f < 0 ? -0.5f : 0.5f;
    float fvar1 = mSampleRate * 0.016666668f < 0 ? -0.5f : 0.5f;
    unsigned int u25 = f7 + mSampleRate * 0.4f;
    unk1c = (f8 + mSampleRate * 0.0015384615f);
    unk20 = (fvar1 + mSampleRate * 0.016666668f);
    unk0.resize(u25);
    unkc.resize(unk0.size());
    unk18 = 0;
    unk3c = 1;
    mPitchDetector = new PitchDetector(unkc, (unk1c + 3U) / 4, unk20 / 4);
    unk30 = 0;
    unk34 = 0;
    unk38 = 0.35f;
    unk2c = unk1c;
    mPeakDetector = new PeakDetector(unk0, unk1c, unk20);
    mVoices.resize(3);
    unk44.resize(mVoices.size());
    unk50.resize(mVoices.size());
    for (int i = 0; i < unk44.size(); i++) {
        unk44[i].resize(0x2000);
        unk50[i] = &unk44[i][0];
    }
    mGranularSynth = new GranularSynth(unk0, mVoices.size(), unk1c, unk20);
    for (int i = 0; i < mVoices.size(); i++) {
    }
    float coeffs[8];
    LowpassCoefficients(coeffs, mSampleRate, 7862.0f, 0.707f);
    mLowPassBiquad = new Biquad(coeffs);
    HighpassCoefficients(coeffs, mSampleRate / 4, 340.0f, 0.707f);
    mHighPassBiquad = new Biquad(coeffs);
    unk78 = 0;
    unk7c = Time2IirA(0.008160001f, mSampleRate / 4);
    SetAttackSmoothing(30);
    SetReleaseSmoothing(80);
}

DSP::Synapse::Synapse::~Synapse() {}

void DSP::Synapse::Synapse::SetVoiceEnabled(unsigned int idx, bool enabled) {
    mGranularSynth->SetVoiceEnabled(idx, enabled);
}

void DSP::Synapse::Synapse::SetVoiceTargetNote(unsigned int idx, float note) {
    mVoices[idx].SetTargetNote(note);
}

void DSP::Synapse::Synapse::SetVoiceGain(unsigned int idx, float gain) {
    mGranularSynth->SetVoiceGain(idx, gain);
}

void DSP::Synapse::Synapse::SetVoiceTransposition(unsigned int idx, float trans) {
    mVoices[idx].SetTransposition(trans);
}

void DSP::Synapse::Synapse::SetVoiceAmount(unsigned int idx, float amt) {
    mVoices[idx].SetAmount(amt);
}

void DSP::Synapse::Synapse::SetVoiceProximityEffect(unsigned int idx, float effect) {
    mVoices[idx].SetProximityEffect(effect);
}

void DSP::Synapse::Synapse::SetVoiceProximityFocus(unsigned int idx, float focus) {
    mVoices[idx].SetProximityFocus(focus);
}

void DSP::Synapse::Synapse::SetAttackSmoothing(float a) {
    float iir = Time2IirA(a / 1000 / (float)unk24, mSampleRate);
    for (int i = 0; i < mVoices.size(); i++) {
        mVoices[i].SetAttackSmoothing(iir);
    }
}

void DSP::Synapse::Synapse::SetReleaseSmoothing(float r) {
    float iir = Time2IirA(r / 1000 / (float)unk24, mSampleRate);
    for (int i = 0; i < mVoices.size(); i++) {
        mVoices[i].SetReleaseSmoothing(iir);
    }
}
