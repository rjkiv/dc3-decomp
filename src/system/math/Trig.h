#pragma once

#define PI 3.1415927f
#define RAD2DEG 57.29578f
#define DEG2RAD 0.01745329238474369049f

float Sine(float);
float FastSin(float);
void TrigTableInit();
void TrigInit();
void TrigTableTerminate();

inline float FastCos(float f) { return FastSin(f + (PI / 2)); }
inline float Cosine(float f) { return Sine(f + (PI / 2)); }
inline float DegreesToRadians(float deg) { return DEG2RAD * deg; }
inline float RadiansToDegrees(float rad) { return RAD2DEG * rad; }

float LimitAng(float);

inline float InterpAng(float f1, float f2, float f3) {
    return f3 * LimitAng(f2 - f1) + f1;
}
