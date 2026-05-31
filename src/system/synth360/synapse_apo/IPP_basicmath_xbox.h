#pragma once

namespace IPP {
    void Add_InPlace(unsigned int, float const *, float *);
    void MulConstant_InPlace(unsigned int, float *, float);
    void Mul_InPlace(unsigned int, float const *, float *);
    void Mul(unsigned int, float const *, float const *, float *);
}
