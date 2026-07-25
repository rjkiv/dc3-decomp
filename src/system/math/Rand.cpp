#include "math/Rand.h"
#include "os/Debug.h"
#include "os/OSFuncs.h"
#include <cmath>

Rand Rand::sRand(0x29A);

Rand::Rand(int i)
    : mRandIndex1(0), mRandIndex2(0), mRandTable(), mSpareGaussianAvailable(0) {
    Seed(i);
}

void Rand::Seed(int seed) {
    int s = seed;
    for (int i = 0; i < DIM(mRandTable); i++) {
        int j = s * 0x41C64E6D + 0x3039;
        s = j * 0x41C64E6D + 0x3039;
        mRandTable[i] = (s & 0x7FFF0000) | (j >> 16);
    }
    mRandIndex1 = 0;
    mRandIndex2 = 0x67;
}

float Rand::Float() { return ((Int() & 0xFFFF) / 65536.0f); }
float Rand::Float(float f1, float f2) { return ((f2 - f1) * Float() + f1); }

int RandomInt() {
    MILO_ASSERT(MainThread(), 0x5C);
    return Rand::sRand.Int();
}

int RandomInt(int i1, int i2) {
    MILO_ASSERT(MainThread(), 0x63);
    return Rand::sRand.Int(i1, i2);
}

float RandomFloat() {
    MILO_ASSERT(MainThread(), 0x69);
    return Rand::sRand.Float();
}

float RandomFloat(float f1, float f2) {
    MILO_ASSERT(MainThread(), 0x6F);
    return Rand::sRand.Float(f1, f2);
}

float Rand::Gaussian() {
    float f2, f3, f5;

    if (mSpareGaussianAvailable) {
        mSpareGaussianAvailable = false;
        return mSpareGaussianValue;
    } else {
        do {
            do {
                f2 = Float(-1.0f, 1.0f);
                f3 = Float(-1.0f, 1.0f);
                f5 = f2 * f2 + f3 * f3;
            } while (f5 >= 1.0f);
        } while (0 == f5);
        f5 = sqrtf(-2.0f * logf(f5) / f5);
        mSpareGaussianValue = f2 * f5;
        mSpareGaussianAvailable = true;
        return f3 * f5;
    }
}

void SeedRand(int seed) { Rand::sRand.Seed(seed); }
