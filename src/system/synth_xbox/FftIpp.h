#pragma once

class FftIpp {
public:
    FftIpp();
    ~FftIpp();
    void SetMode(int);
    void FftReal(float const *__restrict, float *__restrict, float *__restrict);
    void FftRealCcs(float const *__restrict, float *__restrict);

private:
    int unk0;
    int unk4;
    void *unk8; // std::vector<float,class XboxAllocator<float>>*

    int unkc;
    int unk10;
    void *unk14; // std::vector<float,class XboxAllocator<float>>*

    int unk18;
    int unk1c;
    void *unk20; // std::vector<float,class XboxAllocator<float>>*

    int unk24;
    int unk28;
    void *unk2c;

    int unk30;
    int unk34;
    void *unk38; // std::vector<float,class XboxAllocator<float>>*

    int unk3c;
    int unk40;
};
