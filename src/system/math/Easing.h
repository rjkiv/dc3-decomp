#pragma once
#include <cmath>
#include "math/Rot.h"
#include "math/Trig.h"
#include "os/Debug.h"

enum EaseType {
    /** "None": "No Ease" */
    kEaseLinear = 0,
    /** "In": "Non-linear ease in" */
    kEasePolyIn = 1,
    /** "Out": "Non-linear ease out" */
    kEasePolyOut = 2,
    /** "In, then Out": "Non-linear ease in, then out" */
    kEasePolyInOut = 3,
    /** "Out, then In": "Non-linear ease out, then in" */
    kEasePolyOutIn = 4,
    /** "Back In": "Overarching ease in" */
    kEaseBackIn = 5,
    /** "Back Out": "Overarching ease out" */
    kEaseBackOut = 6,
    /** "Back In, then Out": "Overarching ease in, then out" */
    kEaseBackInOut = 7,
    /** "Back Out, then In.": "Overarching ease out, then in" */
    kEaseBackOutIn = 8,
    /** "Bounce In": "Bouncing ease in" */
    kEaseBounceIn = 9,
    /** "Bounce Out": "Bouncing ease out" */
    kEaseBounceOut = 10,
    /** "Bounce In, then Out": "Bouncing ease in, then out" */
    kEaseBounceInOut = 11,
    /** "Bounce Out, then In": "Bouncing ease out, then in" */
    kEaseBounceOutIn = 12,
    /** "Circular In": "Circular ease in" */
    kEaseCircIn = 13,
    /** "Circular Out": "Circular ease out" */
    kEaseCircOut = 14,
    /** "Circular In, then Out": "Circular ease in/out" */
    kEaseCircInOut = 15,
    /** "Circular Out, then In": "Circular ease out/in" */
    kEaseCircOutIn = 16,
    /** "Elastic In": "Overshoots target and returns" */
    kEaseElasticIn = 17,
    /** "Elastic Out": "Overshoots target and returns" */
    kEaseElasticOut = 18,
    /** "Elastic In, then Out": "Overshoots target and returns" */
    kEaseElasticInOut = 19,
    /** "Elastic Out, then In": "Overshoots target and returns" */
    kEaseElasticOutIn = 20,
    /** "Exponential In": "quick ease in" */
    kEaseExpoIn = 21,
    /** "Exponential Out": "quick ease out" */
    kEaseExpoOut = 22,
    /** "Exponential In, then Out": "quick ease in/out" */
    kEaseExpoInOut = 23,
    /** "Exponential Out, then In": "quick ease out/in" */
    kEaseExpoOutIn = 24,
    /** "Sigmoid": "Sigmoidal ease" */
    kEaseSigmoid = 25,
    /** "Sine In": "Sinusoidal ease in" */
    kEaseSineIn = 26,
    /** "Sine Out": "Sinusoidal ease out" */
    kEaseSineOut = 27,
    /** "Sine In, then Out": "Sinusoidal ease in/out" */
    kEaseSineInOut = 28,
    /** "Sine Out, then In": "Sinusoidal ease out/in" */
    kEaseSineOutIn = 29,
    /** "Stair Step": "jerky ease" */
    kEaseStairstep = 30,
    /** "1/3 Stair Step": "jerky ease" */
    kEaseThirdStairstep = 31,
    /** "1/4 Stair Step": "jerky ease" */
    kEaseQuarterStairstep = 32,
    /** "1/2 Stair Step": "jerky ease" */
    kEaseHalfQuarterStairstep = 33,
    /** "1/8 Stair Step": "jerky ease" */
    kEaseQuarterHalfStairstep = 34
};

float EaseLinear(float, float, float);

inline float EasePolyIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 88);
    return pow(t, power);
}

inline float EasePolyOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 95);
    float difference = pow(1.0f - t, power);
    return 1.0f - difference;
}

inline float EasePolyInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 102);
    if (t < 0.5)
        return EasePolyIn(t * 2, power, 0.0f) / 2;
    else
        return (EasePolyOut((t - 0.5f) * 2, power, 0.0f) + 1.0f) / 2;
}

inline float EasePolyOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 112);
    if (t < 0.5)
        return EasePolyOut(t * 2, power, 0.0f) / 2;
    else
        return (EasePolyIn((t - 0.5f) * 2, power, 0.0f) + 1.0f) / 2;
}

float EaseBounceOut(float, float, float);

inline float EaseBounceIn(float t, float, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 123);
    return 1.0f - EaseBounceOut(1.0f - t, 0.0f, 0.0f);
}

inline float EaseBounceInOut(float t, float, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 129);
    if (t < 0.5)
        return EaseBounceIn(t * 2, 0.0f, 0.0f) / 2;
    else
        return (EaseBounceOut(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

inline float EaseBounceOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 137);
    if (t < 0.5)
        return EaseBounceOut(t * 2, 0.0f, 0.0f) / 2;
    else
        return (EaseBounceIn(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

inline float EaseElasticIn(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 145);
    if (t > 0 && t < 1.0f) {
        if (f3 <= 0)
            f3 = 0.45;
    }
    return t;
}

inline float EaseElasticOut(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 164);
    float difference = EaseElasticIn(1.0f - t, power, f3);
    return 1.0f - difference;
}

inline float EaseElasticInOut(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 170);
    if (t < 0.5)
        return EaseElasticIn(t * 2, power, f3) / 2;
    else
        return (EaseElasticOut(t * 2 - 1.0f, power, f3) + 1.0f) / 2;
}

inline float EaseElasticOutIn(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 177);
    if (t < 0.5)
        return EaseElasticOut(t * 2, power, f3) / 2;
    else
        return (EaseElasticIn(t * 2 - 1.0f, power, f3) + 1.0f) / 2;
}

inline float EaseBackIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 184);
    return ((power + 1) * t - power) * t * t;
}

inline float EaseBackOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 190);
    t = 1 - t;
    return -(((power + 1) * t - power) * t * t - 1);
}

inline float EaseBackInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 196);
    if (t < 0.5)
        return EaseBackIn(t * 2, power, 0.0f) / 2;
    else
        return (EaseBackOut(t * 2 - 1.0f, power, 0.0f) + 1) / 2;
}

inline float EaseBackOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 203);
    if (t < 0.5)
        return EaseBackOut(t * 2, power, 0.0f) / 2;
    else
        return (EaseBackIn(t * 2 - 1.0f, power, 0.0f) + 1) / 2;
}

inline float EaseSineIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 210);
    return 1.0f - FastSin((t + 1) * 1.570796370506287f);
}

inline float EaseSineOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 216);
    return FastSin((t + 1) * 1.570796370506287f);
}

inline float EaseSineInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 222);
    return (FastCos(t * PI) - 1.0f) * -0.5f;
}

inline float EaseSineOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 228);
    if (t < 0.5)
        return EaseSineOut(t * 2, 0.0f, 0.0f);
    else
        return EaseSineIn(t * 2 - 1.0f, 0.0f, 0.0f);
}

inline float EaseExpoIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 236);
    if (t == 0.0f)
        return t;
    else {
        float ret = pow(2, (t - 1) * 10);
        return ret - 0.001f;
    }
}

inline float EaseExpoOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 242);
    if (t == 1.0f)
        return t;
    else {
        float ret = pow(2, t * -10);
        return (1.0f - ret) * 1.001f;
    }
}

inline float EaseExpoInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 248);
    if (t != 0.0f && t != 1.0f) {
        if (t < 0.5) {
            float ret = pow(2, (t * 2.0f - 1.0f) * 10);
            return ret * 0.5 - 0.005f;
        } else {
            float ret = pow(2, (t * 2.0f - 1.0f) * -10);
            return (1.0f - ret) * 0.50025f;
        }
    } else
        return t;
}

inline float EaseExpoOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 258);
    if (t < 0.5)
        return EaseExpoOut(t * 2, 0.0f, 0.0f) * 0.5f;
    else
        return (EaseExpoIn(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

inline float EaseCircIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 266);
    float ret = -(t * t - 1.0f);
    ret = sqrtf(ret) - 1.0f;
    return -ret;
}

inline float EaseSigmoid(float t, float, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 0x51);
    float ret = (t * t * 3.0f) - (t * t * t * 2.0f);
    if (ret < 0)
        return 0;
    if (ret > 1.0f)
        return 1.0f;
}

inline float EaseInExp(float t) {
    MILO_ASSERT(t >= 0 && t <= 1, 0x39);
    return std::pow(t, 3.03);
}

inline float EaseCircOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 272);
    float ret = t - 1;
    ret = -(ret * ret - 1.0f);
    return sqrt(ret);
}

inline float EaseCircInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 278);
    float ret;
    float tmp_f13 = 0.5f;
    if (t < 0.5f) {
        tmp_f13 = -0.5f;
        t *= t;
        ret = -(t * 4.0f - 1.0f);
        ret = sqrtf(ret) - 1.0f;
    } else {
        ret = t * 2.0f - 2.0f;
        ret = -(ret * ret - 1.0f);
        ret = sqrtf(ret) + 1.0f;
    }
    return ret * tmp_f13;
}

inline float EaseCircOutIn(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 286);
    if (t < 0.5)
        return EaseCircOut(t * 2, 0.0f, 0.0f);
    else
        return (EaseCircIn(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

inline float EaseStairstep(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 294);
    if (f3 == 0) {
        f3 = 2;
    }
    float tmp_f30 = t * f3;
    float tmp_f26 = floor(tmp_f30);
    f3 = 1.0f / f3; // this is SUPPOSED to be here, but it's getting scheduled for later
    return (EasePolyInOut(tmp_f30 - tmp_f26, power, 0.0f) + tmp_f26) * f3;
}

inline float EaseThirdStairstep(float t, float power, float) {
    return EaseStairstep(t, power, 3);
}

inline float EaseQuarterStairstep(float t, float power, float) {
    return EaseStairstep(t, power, 4);
}

inline float EaseHalfQuarterStairstep(float t, float power, float) {
    if (t < 0.5) {
        return EaseStairstep(t, power, 2);
    } else {
        return EaseStairstep(t, power, 4);
    }
}

inline float EaseQuarterHalfStairstep(float t, float power, float) {
    if (t < 0.5) {
        return EaseStairstep(t, power, 4);
    } else {
        return EaseStairstep(t, power, 2);
    }
}

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
