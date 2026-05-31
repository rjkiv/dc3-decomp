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
    unsigned int unk0;
    unsigned int unk4;
    aligned_vector<float> unk8;
    aligned_vector<float> unk14;
    aligned_vector<float> unk20;
    aligned_vector<float> unk2c;
    aligned_vector<float> unk38;
};
