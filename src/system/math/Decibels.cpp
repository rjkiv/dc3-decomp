#include "os/Debug.h"
#include "math/Decibels.h"
#include <cmath>

float DbToRatio(float db) {
    if (db <= -96.0f)
        return 0.0f;
    else
        return pow(10.0f, db / 20.0f);
}

float RatioToDb(float ratio) {
    static const float zero = 0.0f;
    MILO_ASSERT(ratio >= 0.0f, 0x1B);
    return (ratio <= zero) ? -96.0f : (float)log10(ratio) * 20.0f;
}
