#pragma once
#include "synth360/synapse_apo/common_vector.h"

// size 0x44
class FftIpp {
public:
    FftIpp();
    ~FftIpp();
    void SetMode(int);
    void FftReal(float const *__restrict, float *__restrict, float *__restrict);
    void FftRealCcs(float const *__restrict, float *__restrict);

private:
    int m_sFFT; // 0x0
    unsigned int m_uOrder; // 0x4
    aligned_vector<float> m_fReal; // 0x8
    aligned_vector<float> m_fImaginary; // 0x14
    aligned_vector<float> m_fTemp; // 0x20
    aligned_vector<float> m_fUnityTable; // 0x2c
    aligned_vector<float> m_fSinCosTable; // 0x38
};
