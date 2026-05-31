#pragma once
#include "synth360/synapse_apo/common_vector.h"
#include "synth360/synapse_apo/FftIpp.h"

namespace DSP {
    class SpectralAnalysis {
    public:
        void Analyze(const float *, float *);
        void SetMode(unsigned int, unsigned int);

        unsigned int GetUnk0() const { return unk0; }

    private:
        unsigned int unk0;
        unsigned int unk4;
        int unk8;
        FftIpp unkc;
        FftIpp unk50;
        aligned_vector<float> unk94;
        aligned_vector<float> unka0;
        aligned_vector<float> unkac;
        aligned_vector<float> unkb8;
        aligned_vector<float> unkc4;
        aligned_vector<float> unkd0;
    };
}
