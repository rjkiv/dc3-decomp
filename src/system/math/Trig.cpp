#include "math/Trig.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include <cmath>

struct SinEntry {
    float val; // 0x0
    float grad; // 0x4
};

static SinEntry gBigSinTable[512];

void TrigTableInit() {
    int i;
    for (i = 0; i < 256; i++) {
        SinEntry *entry = &gBigSinTable[i];
        entry->val = sinf(0.024543693f * i);
        if (i != 0) {
            entry[-1].grad = entry[0].val - entry[-1].val;
        }
    }
    gBigSinTable[i - 1].grad = sinf(0.024543693f * i) - gBigSinTable[i - 1].val;
}

void TrigTableTerminate() {}

float Lookup(float arg8) {
    float x = arg8 * 40.743664f;
    int temp_r5 = (int)x;
    unsigned char idx = temp_r5;
    SinEntry &entry = gBigSinTable[idx];
    float res = x - (float)temp_r5;
    return (res * entry.grad) + entry.val;
}

float Sine(float arg8) {
    if (arg8 < 0.0f) {
        return -Lookup(-arg8);
    } else
        return Lookup(arg8);
}

float FastSin(float f) {
    if (f < 0.0f) {
        return -gBigSinTable[((int)(-40.743664f * f + 0.49999f) & 0xFF)].val;
    } else
        return gBigSinTable[((int)(40.743664f * f + 0.49999f) & 0xFF)].val;
}

static DataNode DataSin(DataArray *a) { return sinf(DegreesToRadians(a->Float(1))); }
static DataNode DataCos(DataArray *da) { return cosf(DegreesToRadians(da->Float(1))); }
static DataNode DataTan(DataArray *da) { return tanf(DegreesToRadians(da->Float(1))); }

static DataNode DataASin(DataArray *da) {
    float f = da->Float(1);
    if (IsNaN(f))
        return 0.0f;
    else
        return RadiansToDegrees(asinf(f));
}

static DataNode DataACos(DataArray *da) {
    float f = da->Float(1);
    if (IsNaN(f))
        return 0.0f;
    else
        return RadiansToDegrees(acosf(f));
}

static DataNode DataATan(DataArray *da) {
    float f = da->Float(1);
    if (IsNaN(f))
        return 0.0f;
    else
        return RadiansToDegrees(atanf(f));
}

void TrigInit() {
    DataRegisterFunc("sin", DataSin);
    DataRegisterFunc("cos", DataCos);
    DataRegisterFunc("tan", DataTan);
    DataRegisterFunc("asin", DataASin);
    DataRegisterFunc("acos", DataACos);
    DataRegisterFunc("atan", DataATan);
}
