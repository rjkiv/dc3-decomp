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
float EasePolyInOut(float, float, float);
float EasePolyOutIn(float, float, float);
float EaseBounceIn(float, float, float);
float EaseBounceOut(float, float, float);
float EaseBounceInOut(float, float, float);
float EaseBounceOutIn(float, float, float);
float EaseElasticIn(float, float, float);
float EaseElasticOut(float, float, float);
float EaseElasticInOut(float, float, float);
float EaseElasticOutIn(float, float, float);
float EaseBackIn(float, float, float);
float EaseBackOut(float, float, float);
float EaseBackInOut(float, float, float);
float EaseBackOutIn(float, float, float);
float EaseSineIn(float, float, float);
float EaseSineOut(float, float, float);
float EaseSineInOut(float, float, float);
float EaseSineOutIn(float, float, float);
float EaseExpoIn(float, float, float);
float EaseExpoOut(float, float, float);
float EaseExpoInOut(float, float, float);
float EaseExpoOutIn(float, float, float);
float EaseCircIn(float, float, float);
float EaseCircOut(float, float, float);
float EaseCircInOut(float, float, float);
float EaseCircOutIn(float, float, float);
float EaseStairstep(float, float, float);
float EaseThirdStairstep(float, float, float);
float EaseQuarterStairstep(float, float, float);
float EaseHalfQuarterStairstep(float, float, float);
float EaseQuarterHalfStairstep(float, float, float);

typedef float EaseFunc(float, float, float);

EaseFunc *gEaseFuncs[35] = {
    EaseLinear,
    EasePolyIn,
    EasePolyOut,
    EasePolyInOut,
    EasePolyOutIn,
    EaseBounceIn,
    EaseBounceOut,
    EaseBounceInOut,
    EaseBounceOutIn,
    EaseElasticIn,
    EaseElasticOut,
    EaseElasticInOut,
    EaseElasticOutIn,
    EaseBackIn,
    EaseBackOut,
    EaseBackInOut,
    EaseBackOutIn,
    EaseSineIn,
    EaseSineOut,
    EaseSineInOut,
    EaseSineOutIn,
    EaseExpoIn,
    EaseExpoOut,
    EaseExpoInOut,
    EaseExpoOutIn,
    EaseCircIn,
    EaseCircOut,
    EaseCircInOut,
    EaseCircOutIn,
    EaseStairstep,
    EaseThirdStairstep,
    EaseQuarterStairstep,
    EaseHalfQuarterStairstep,
    EaseQuarterHalfStairstep,
};

inline EaseFunc *GetEaseFunction(EaseType e) {
    MILO_ASSERT(e >= kEaseLinear && e <= kEaseQuarterHalfStairstep, 0x16B);
    return gEaseFuncs[e];
}
