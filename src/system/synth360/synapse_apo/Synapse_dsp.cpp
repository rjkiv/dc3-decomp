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

DSP::Synapse::Synapse::Synapse(float f1) : unk24(64), unk6c(f1) {
    float f7 = unk6c * 0.4f < 0 ? -0.5f : 0.5f;
    float f8 = unk6c * 0.0015384615f < 0 ? -0.5f : 0.5f;
    float fvar1 = unk6c * 0.016666668f < 0 ? -0.5f : 0.5f;
    int u25 = f7 + unk6c * 0.4f;
    unk1c = (f8 + unk6c * 0.0015384615f);
    unk20 = (fvar1 + unk6c * 0.016666668f);
    unk0.resize(u25);
    unkc.resize(unk0.size());
    unk18 = 0;
    unk3c = 1;
    unk28 = new PitchDetector(unkc, (unk1c + 3U) >> 2, unk20 >> 2);
    unk30 = 0;
    unk34 = 0;
    unk38 = 0.35f;
    unk2c = unk1c;
    unk40 = new PeakDetector(unk0, unk1c, unk20);
    unk5c.resize(3);
    unk44.resize(unk5c.size());
    unk50.resize(unk5c.size());
    for (int i = 0; i < unk44.size(); i++) {
        unk44[i].resize(0x2000);
        unk50[i] = &unk44[i][0];
    }
    unk68 = new GranularSynth(unk0, unk5c.size(), unk1c, unk20);
    for (int i = 0; i < unk5c.size(); i++) {
    }
    float coeffs[8];
    LowpassCoefficients(coeffs, unk6c, 7862.0f, 0.707f);
    unk70 = new Biquad(coeffs);
    HighpassCoefficients(coeffs, unk6c / 4, 340.0f, 0.707f);
    unk74 = new Biquad(coeffs);
    unk78 = 0;
    unk7c = Time2IirA(0.008160001f, unk6c / 4);
    SetAttackSmoothing(30);
    SetReleaseSmoothing(80);
}

DSP::Synapse::Synapse::~Synapse() {}

void DSP::Synapse::Synapse::SetVoiceEnabled(unsigned int ui1, bool b2) {
    unk68->SetVoiceEnabled(ui1, b2);
}

void DSP::Synapse::Synapse::SetVoiceTargetNote(unsigned int idx, float note) {
    unk5c[idx].SetTargetNote(note);
}
