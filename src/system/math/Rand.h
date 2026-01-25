#pragma once

class Rand {
public:
    Rand(int);
    void Seed(int);
    int Int();
    int Int(int, int);
    int FastInt(int, int);
    float Float();
    float Float(float, float);
    float Gaussian();

    static Rand sRand;

private:
    unsigned int mRandIndex1;
    unsigned int mRandIndex2;
    unsigned int mRandTable[256];
    float mSpareGaussianValue;
    bool mSpareGaussianAvailable;
};

void SeedRand(int);
int RandomInt();
int RandomInt(int, int);
float RandomFloat();
float RandomFloat(float, float);
