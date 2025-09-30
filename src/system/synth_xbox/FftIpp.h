#pragma once

class FftIpp {
public:
    void FftRealCcs(unsigned int *, volatile float &, unsigned int *, float &);
    void
    FftReal(unsigned int *, volatile float &, unsigned int *, float &, volatile float &);
    ~FftIpp();
    FftIpp();
    void SetMode(int);

    int unk0;
    int unk4;
    int unk8;
    int unkc;
    int unk10;
    int unk14;
    int unk18;
    int unk1c;
    int unk20;
    int unk24;
    int unk28;
    int unk2c;
    int unk30;
    int unk34;
    int unk38;
    int unk3c;
    int unk40;
};
