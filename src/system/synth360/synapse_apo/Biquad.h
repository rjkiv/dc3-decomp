#pragma once

namespace DSP {
    // size 0x1c
    class Biquad {
    public:
        Biquad(float *);
        void SetCoefficients(float *);
        float coefs[7];
    };
};
