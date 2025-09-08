#pragma once

float Sine(float);
float FastSin(float);
void TrigTableInit();
void TrigInit();

inline float FastCos(float f) { return FastSin(f + 1.570796370506287f); }
inline float Cosine(float f) { return Sine(f + 1.5707964f); }
inline float DegreesToRadians(float deg) { return 0.017453292f * deg; }
inline float RadiansToDegrees(float rad) { return 57.295776f * rad; }

float LimitAng(float);
