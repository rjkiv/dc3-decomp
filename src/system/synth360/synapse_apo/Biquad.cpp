#include "synth360/synapse_apo/Biquad.h"

namespace DSP {
    void Biquad::SetCoefficients(float *flts) {
        if (flts == nullptr) {
            m_fCoeffs[4] = 0.0f;
            m_fCoeffs[3] = 0.0f;
            m_fCoeffs[2] = 0.0f;
            m_fCoeffs[1] = 0.0f;
            m_fCoeffs[0] = 1.0f;
        } else {
            m_fCoeffs[0] = flts[0] / flts[3];
            m_fCoeffs[1] = flts[1] / flts[3];
            m_fCoeffs[2] = flts[2] / flts[3];
            m_fCoeffs[3] = flts[4] / flts[3];
            m_fCoeffs[4] = flts[5] / flts[3];
        }
        m_fW[1] = 0.0f;
        m_fW[0] = 0.0f;
    }
    Biquad::Biquad(float *flts) { SetCoefficients(flts); }
};
