#include "math/Trig.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include <cmath>

float gBigSinTable[0x200];

void TrigTableInit() {
    float *temp_r30 = gBigSinTable;
    int i;
    for (i = 0; i < 256; i++, temp_r30 += 2) {
        *temp_r30 = std::sin(0.024543693f * i);
        if (i != 0) {
            *(temp_r30 - 1) = *temp_r30 - *(temp_r30 - 2);
        }
    }
    int tmp = (i - 1) * 2;
    *(gBigSinTable + tmp + 1) = std::sin(0.024543693f * i) - *(gBigSinTable + tmp);
}

void TrigTableTerminate() {}

inline float Lookup(float arg8) {
    float x = arg8 * 40.743664f;
    int temp_r5 = (int)x;
    int idx = (temp_r5 & 0xFF) * 2;
    float *offset = &gBigSinTable[idx];
    float res = x - (float)temp_r5;
    return (res * offset[1]) + offset[0];
}

float Sine(float arg8) {
    if (arg8 < 0.0f) {
        return -Lookup(-arg8);
    } else
        return Lookup(arg8);
}

float FastSin(float f) {
    if (f < 0.0f) {
        return -gBigSinTable[((int)(-40.743664f * f + 0.49999f) & 0xFF) * 2];
    } else
        return gBigSinTable[((int)(40.743664f * f + 0.49999f) & 0xFF) * 2];
}

DataNode DataSin(DataArray *a) { return (float)sin(DegreesToRadians(a->Float(1))); }
DataNode DataCos(DataArray *da) { return std::cos(DegreesToRadians(da->Float(1))); }
DataNode DataTan(DataArray *da) { return std::tan(DegreesToRadians(da->Float(1))); }

DataNode DataASin(DataArray *da) {
    float f = da->Float(1);
    if (IsNaN(f))
        return 0.0f;
    else
        return RadiansToDegrees(std::asin(f));
}

DataNode DataACos(DataArray *da) {
    float f = da->Float(1);
    if (IsNaN(f))
        return 0.0f;
    else
        return RadiansToDegrees(std::acos(f));
}

DataNode DataATan(DataArray *da) {
    float f = da->Float(1);
    if (IsNaN(f))
        return 0.0f;
    else
        return RadiansToDegrees(std::atan(f));
}

void TrigInit() {
    DataRegisterFunc("sin", DataSin);
    DataRegisterFunc("cos", DataCos);
    DataRegisterFunc("tan", DataTan);
    DataRegisterFunc("asin", DataASin);
    DataRegisterFunc("acos", DataACos);
    DataRegisterFunc("atan", DataATan);
}
