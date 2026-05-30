#pragma once

namespace DSP {
    class Biquad {
    public:
        Biquad(float *);
        void SetCoefficients(float *);
        float coefs[7];
    };
};
