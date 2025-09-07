#pragma once

#include "os/Debug.h"
enum EaseType {
    kEaseLinear = 0,
    kEasePolyIn = 1,
    kEasePolyOut = 2,
    kEasePolyInOut = 3,
    kEasePolyOutIn = 4,
    kEaseBackIn = 5,
    kEaseBackOut = 6,
    kEaseBackInOut = 7,
    kEaseBackOutIn = 8,
    kEaseBounceIn = 9,
    kEaseBounceOut = 10,
    kEaseBounceInOut = 11,
    kEaseBounceOutIn = 12,
    kEaseCircIn = 13,
    kEaseCircOut = 14,
    kEaseCircInOut = 15,
    kEaseCircOutIn = 16,
    kEaseElasticIn = 17,
    kEaseElasticOut = 18,
    kEaseElasticInOut = 19,
    kEaseElasticOutIn = 20,
    kEaseExpoIn = 21,
    kEaseExpoOut = 22,
    kEaseExpoInOut = 23,
    kEaseExpoOutIn = 24,
    kEaseSigmoid = 25,
    kEaseSineIn = 26,
    kEaseSineOut = 27,
    kEaseSineInOut = 28,
    kEaseSineOutIn = 29,
    kEaseStairstep = 30,
    kEaseThirdStairstep = 31,
    kEaseQuarterStairstep = 32,
    kEaseHalfQuarterStairstep = 33,
    kEaseQuarterHalfStairstep = 34
};

float EaseLinear(float, float, float);
float EasePolyIn(float, float, float);
float EasePolyOut(float, float, float);
// todo: the rest

typedef float EaseFunc(float, float, float);

EaseFunc *gEaseFuncs[35] = { EaseLinear, EasePolyIn, EasePolyOut };

inline EaseFunc *GetEaseFunction(EaseType e) {
    MILO_ASSERT(e >= kEaseLinear && e <= kEaseQuarterHalfStairstep, 0x16B);
    return gEaseFuncs[e];
}
