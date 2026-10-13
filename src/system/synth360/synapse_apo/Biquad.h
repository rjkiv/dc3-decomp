#pragma once

namespace DSP {
    // size 0x1c
    class Biquad {
    public:
        Biquad(float *);
        void SetCoefficients(float *);

    private:
        float m_fCoeffs[5]; // 0x0
        float m_fW[2]; // 0x14
    };
};
