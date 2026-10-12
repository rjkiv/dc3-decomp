#pragma once
#include "synth360/synapse_apo/common_vector.h"
#include "synth360/synapse_apo/FftIpp.h"

namespace DSP {
    class SpectralAnalysis {
    public:
        void Analyze(const float *, float *);
        void SetMode(unsigned int, unsigned int);

        unsigned int GetUnk0() const { return m_sWindowLen; }

    protected:
        unsigned int m_sWindowLen; // 0x0
        unsigned int m_sFft; // 0x4
        unsigned int m_nBins; // 0x8
        FftIpp m_fft; // 0xc
        FftIpp m_fft2; // 0x50
        aligned_vector<float> m_aSig; // 0x94
        aligned_vector<float> m_aSp; // 0xa0
        aligned_vector<float> m_fSines; // 0xac
        aligned_vector<float> m_fCosines; // 0xb8
        aligned_vector<float> m_aSpReal; // 0xc4
        aligned_vector<float> m_aSpImag; // 0xd0
        double m_fSum; // 0xe0
    };
}
