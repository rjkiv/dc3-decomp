#include "synth/WahEffect.h"
#include "os/Debug.h"
#include "math/Utl.h"
#include <cmath>

WahEffect::WahEffect(IXAudioBatchAllocator *) {
    unk2c = 96000;
    unk0 = 7;
    unk4 = 1000.0f;
    unk8 = 5000.0f;
    unkc = 1.35f;
    unk10 = 0.3f;
    unk14 = -1.0f;
    unk18 = 0.5f;
    unk1c = 1.0f;
    unk20 = 0.5f;
    unk24 = 0;
    unk28 = 1e+30;
    unk30 = 0;
    unk44 = 0;
    unk48 = 0;
    unk40 = 0;
    unk3c = 0;
    unk38 = 0;
    unk34 = 0;
}

void WahEffect::Reset() {
    unk30 = 0;
    unk38 = 0;
    unk34 = 0;
    unk40 = 0;
    unk3c = 0;
}

void WahEffect::SetParameters(WahEffect::Params const &params) {
    unk0 = params.unk4;
    unk8 = params.unk8;
    unk4 = params.unkc;
    unkc = params.unk10;
    unk10 = params.unk14;
    unk14 = params.unk18;
    unk18 = params.unk1c;
    unk1c = params.unk20;
    unk20 = params.unk24;
}

void WahEffect::Process(float *buf, int numSamples, int numChans) {
    MILO_ASSERT(numChans <= 2, 0x34);

    // Load parameters in target order
    float f0_unk18 = unk18;
    float f10 = unk4;
    float f9 = unk8;
    float f8 = unk10;
    float f13_unk14 = unk14;

    // Constants
    float f26 = 2.0f;
    float f31 = 1.0f;
    float f7 = f0_unk18 * f26;        // sweepRange * 2
    float f6 = f31 - f0_unk18;        // 1 - sweepRange
    float f0_norm = 4.1666666e-5f;    // 1/24000
    float f12_twopi = 6.2831853f;     // 2*PI
    float f25 = f10 * f0_norm;        // freqLo normalized
    float f24 = f9 * f0_norm;         // freqHi normalized
    float f23 = f8 * f12_twopi;       // resonance * 2*PI

    // Load state variables BEFORE the comparison
    float f10_state = unk34;
    float f0_state = unk38;
    float f12_state = unk3c;
    float f11_state = unk40;
    float f27 = unk30;
    float f30 = 0.5f;

    // Copy state to stack arrays
    float stack50[2];
    float stack58[2];
    stack50[0] = f10_state;
    stack50[1] = f0_state;

    // Compute phase rate
    float f21 = f7 / f6;

    stack58[0] = f12_state;
    stack58[1] = f11_state;

    // Compute sweep value based on unk14 - comparison happens AFTER state loading
    float sweepVal;
    if (f13_unk14 < 0.0f) {
        sweepVal = f31;
    } else {
        // Phase modulation with Mod
        float f0_invtwopi = 0.15915494f;  // 1/(2*PI)
        float f2 = f31;
        float f0_neghalf = -0.5f;
        float tmp = f27 * f0_invtwopi - f13_unk14;
        tmp = tmp + f30;
        float modPhase = Mod(tmp - f0_neghalf, f2);
        modPhase = modPhase - f30;

        // fsel clamp
        float f0_lo = 0.9f;
        float f13_hi = 1.1f;
        sweepVal = (modPhase >= 0.0f) ? f0_lo : f13_hi;
    }

    // Apply resonance scaling
    float f13_scaled = unkc * sweepVal;
    float f0_unk0 = unk0;
    float f0_scale = 1.3089970e-4f;   // 0x3909421f
    float f18 = f13_scaled * f0_scale;

    // Clamp unk0 to >= 1.0
    if (f0_unk0 < f31) {
        unk0 = f31;
    }

    // Process samples
    if (numSamples > 0) {
        float f19 = 0.99999f;         // 0x3f7fff58
        float f20 = -4.2704245e-9f;   // 0xb192bb0d (negative)
        float f22 = 0.99958f;         // 0x3f7fe47a

        int sampleOffset = 0;

        do {
            // Compute sin of phase
            float sinVal = sin(f27);
            sinVal = (float)sinVal;
            float f13_unk1c = unk1c;
            float f12_coef = f22;

            // Compute sweep
            float f0_sweep = (sinVal + f31) * f30;

            // Check unk1c
            if (f13_unk1c < f30) {
                f0_sweep = unk20;
            } else {
                float f11_unk28 = unk28;
                if (f11_unk28 != f13_unk1c) {
                    unk2c = 0;
                }
            }

            // Counter interpolation
            int counter = unk2c;
            if (counter <= 96000) {
                int nextCounter = counter + 1;
                float counterF = (float)counter;
                float prod = counterF * f20;
                f12_coef = prod + f19;
                unk2c = nextCounter;
            }

            // Frequency tracking
            float f11_unk24 = unk24;
            float f11_diff = f11_unk24 - f0_sweep;
            unk28 = f13_unk1c;
            float f13_unk0 = unk0;
            float newFreq = f11_diff * f12_coef + f0_sweep;
            unk24 = newFreq;

            // Filter coefficients
            float f12_inv = f31 - newFreq;
            float blend = newFreq * f25;
            blend = f12_inv * f24 + blend;
            float filterFreq = blend * f30 + f23;
            float f28 = blend;
            float filterDiv = filterFreq / f13_unk0;
            float f17 = f31 - filterDiv;

            // Compute cos/sin for filter
            float cosVal = cos(f28);
            cosVal = (float)cosVal;
            float f29 = f17 * f17;
            float cosMod = cosVal * f17;
            float f28_scaled = cosMod * f26;

            float sinVal2 = sin(f28);
            float feedback = (f31 - f17) * f13_unk0;
            sinVal2 = (float)sinVal2;
            feedback = feedback * sinVal2;

            // Process channels
            if (numChans > 0) {
                float f13_gain = f21 + f31;
                float *bufPtr = buf + sampleOffset;

                for (int ch = 0; ch < numChans; ch++) {
                    float sample = bufPtr[ch];

                    float state1 = stack50[ch];
                    unk44 = sample;
                    float state2 = stack58[ch];

                    stack58[ch] = state1;

                    // Biquad filter
                    float tmp1 = sample * feedback;
                    tmp1 = state1 * f28_scaled + tmp1;
                    tmp1 = tmp1 - state2 * f29;
                    stack50[ch] = tmp1;

                    // Soft clip
                    float out = sample * f30 + tmp1;
                    float absOut = fabs(out);
                    out = f13_gain * out;
                    absOut = absOut * f21 + f31;
                    out = out / absOut;

                    unk48 = out;
                    bufPtr[ch] = out;
                }
            }

            // Update phase
            numSamples--;
            f27 = f18 + f27;
            sampleOffset += numChans;
        } while (numSamples != 0);
    }

    // Store phase - compare f27 with 2*PI, subtract if greater
    unk30 = f27;
    if (f27 > f12_twopi) {
        unk30 = f27 - f12_twopi;
    }

    // Copy state back from stack
    float *dest = &unk3c;
    for (int i = 0; i < 2; i++) {
        float s1 = stack50[i];
        float s2 = stack58[i];
        dest[-1] = s1;  // Write to unk34/unk38
        *dest = s2;     // Write to unk3c/unk40
        dest++;
    }
}
