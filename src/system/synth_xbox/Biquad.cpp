#include "synth_xbox/Biquad.h"
#include "Biquad.h"

namespace DSP {
    void Biquad::SetCoefficients(float *flts) {
        if (flts == nullptr) {
            coefs[4] = 0.0f;
            coefs[3] = 0.0f;
            coefs[2] = 0.0f;
            coefs[1] = 0.0f;
            coefs[0] = 1.0f;
        } else {
            coefs[0] = flts[0] / flts[3];
            coefs[1] = flts[1] / flts[3];
            coefs[2] = flts[2] / flts[3];
            coefs[3] = flts[4] / flts[3];
            coefs[4] = flts[5] / flts[3];
        }
        coefs[6] = 0.0f;
        coefs[5] = 0.0f;
    }
    Biquad::Biquad(float *flts) { SetCoefficients(flts); }
};
