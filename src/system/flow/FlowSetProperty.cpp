#include "math/Easing.h"
#include "math/Trig.h"
#include <cmath>
#include "math/Rot.h"

// these defs are supposed to be in Easing.h but thog dont caare

float EasePolyIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 88);
    return pow(t, power);
}

float EasePolyOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 95);
    float difference = pow(1.0f - t, power);
    return 1.0f - difference;
}

float EasePolyInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 102);
    if (t < 0.5)
        return EasePolyIn(t * 2, power, 0.0f) / 2;
    else
        return (EasePolyOut((t - 0.5f) * 2, power, 0.0f) + 1.0f) / 2;
}

float EasePolyOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1 && power != 0, 112);
    if (t < 0.5)
        return EasePolyOut(t * 2, power, 0.0f) / 2;
    else
        return (EasePolyIn((t - 0.5f) * 2, power, 0.0f) + 1.0f) / 2;
}

float EaseBounceIn(float t, float, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 123);
    return 1.0f - EaseBounceOut(1.0f - t, 0.0f, 0.0f);
}

float EaseBounceInOut(float t, float, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 129);
    if (t < 0.5)
        return EaseBounceIn(t * 2, 0.0f, 0.0f) / 2;
    else
        return (EaseBounceOut(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

float EaseBounceOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 137);
    if (t < 0.5)
        return EaseBounceOut(t * 2, 0.0f, 0.0f) / 2;
    else
        return (EaseBounceIn(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

float EaseElasticIn(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 145);
    if (t > 0 && t < 1.0f) {
        if (f3 <= 0)
            f3 = 0.45;
    } else {
        return t;
    }
}

float EaseElasticOut(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 164);
    float difference = EaseElasticIn(1.0f - t, power, f3);
    return 1.0f - difference;
}

float EaseElasticInOut(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 170);
    if (t < 0.5)
        return EaseElasticIn(t * 2, power, f3) / 2;
    else
        return (EaseElasticOut(t * 2 - 1.0f, power, f3) + 1.0f) / 2;
}

float EaseElasticOutIn(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 177);
    if (t < 0.5)
        return EaseElasticOut(t * 2, power, f3) / 2;
    else
        return (EaseElasticIn(t * 2 - 1.0f, power, f3) + 1.0f) / 2;
}

float EaseBackIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 184);
    return ((power + 1) * t - power) * t * t;
}

float EaseBackOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 190);
    t = 1 - t;
    return -(((power + 1) * t - power) * t * t - 1);
}

float EaseBackInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 196);
    if (t < 0.5)
        return EaseBackIn(t * 2, power, 0.0f) / 2;
    else
        return (EaseBackOut(t * 2 - 1.0f, power, 0.0f) + 1) / 2;
}

float EaseBackOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 203);
    if (t < 0.5)
        return EaseBackOut(t * 2, power, 0.0f) / 2;
    else
        return (EaseBackIn(t * 2 - 1.0f, power, 0.0f) + 1) / 2;
}

float EaseSineIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 210);
    return 1.0f - FastSin((t + 1) * 1.570796370506287f);
}

float EaseSineOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 216);
    return FastSin((t + 1) * 1.570796370506287f);
}

float EaseSineInOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 222);
    return (FastCos(t * PI) - 1.0f) * -0.5f;
}

float EaseSineOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 228);
    if (t < 0.5)
        return EaseSineOut(t * 2, 0.0f, 0.0f);
    else
        return EaseSineIn(t * 2 - 1.0f, 0.0f, 0.0f);
}

float EaseExpoIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 236);
    if (t == 0.0f)
        return t;
    else {
        float ret = pow(2, (t - 1) * 10);
        return ret - 0.001f;
    }
}

float EaseExpoOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 242);
    if (t == 1.0f)
        return t;
    else {
        float ret = pow(2, t * -10);
        return (1.0f - ret) * 1.001f;
    }
}

float EaseExpoInOut(float t, float power, float) {
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

float EaseExpoOutIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 258);
    if (t < 0.5)
        return EaseExpoOut(t * 2, 0.0f, 0.0f) * 0.5f;
    else
        return (EaseExpoIn(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

float EaseCircIn(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 266);
    float ret = -(t * t - 1.0f);
    ret = sqrtf(ret) - 1.0f;
    return -ret;
}

float EaseCircOut(float t, float power, float) {
    MILO_ASSERT(t >= 0 && t <= 1, 272);
    float ret = t - 1;
    ret = -(ret * ret - 1.0f);
    return sqrt(ret);
}

float EaseCircInOut(float t, float power, float) {
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

float EaseCircOutIn(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 286);
    if (t < 0.5)
        return EaseCircOut(t * 2, 0.0f, 0.0f);
    else
        return (EaseCircIn(t * 2 - 1.0f, 0.0f, 0.0f) + 1.0f) / 2;
}

float EaseStairstep(float t, float power, float f3) {
    MILO_ASSERT(t >= 0 && t <= 1, 294);
    if (f3 == 0) {
        f3 = 2;
    }
    float tmp_f30 = t * f3;
    float tmp_f26 = floor(tmp_f30);
    f3 = 1.0f / f3; // this is SUPPOSED to be here, but it's getting scheduled for later
    return (EasePolyInOut(tmp_f30 - tmp_f26, power, 0.0f) + tmp_f26) * f3;
}

float EaseThirdStairstep(float t, float power, float) {
    return EaseStairstep(t, power, 3);
}

float EaseQuarterStairstep(float t, float power, float) {
    return EaseStairstep(t, power, 4);
}

float EaseHalfQuarterStairstep(float t, float power, float) {
    if (t < 0.5) {
        return EaseStairstep(t, power, 2);
    } else {
        return EaseStairstep(t, power, 4);
    }
}

float EaseQuarterHalfStairstep(float t, float power, float) {
    if (t < 0.5) {
        return EaseStairstep(t, power, 4);
    } else {
        return EaseStairstep(t, power, 2);
    }
}
