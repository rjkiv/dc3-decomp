#pragma once

class DeJitter {
public:
    DeJitter();
    void Reset();
    float NewMs(float, float &);

    int unk80;
    int unk84;
    float unk88;
    float unk8c;
};
