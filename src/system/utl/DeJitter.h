#pragma once

class DeJitter {
public:
    DeJitter();
    void Reset();
    float NewMs(float, float &);

    char filler[0x80];
    int unk80;
    int unk84;
    float unk88;
    float unk8c;
};
