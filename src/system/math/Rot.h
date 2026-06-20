#pragma once
#include "math/Mtx.h"
#include "math/Trig.h"
#include "math/Vec.h"

TextStream &operator<<(TextStream &ts, const Hmx::Quat &v);

float GetXAngle(const Hmx::Matrix3 &);
float GetYAngle(const Hmx::Matrix3 &);
float GetZAngle(const Hmx::Matrix3 &);

void MakeEuler(const Hmx::Matrix3 &, Vector3 &);
void MakeScale(const Hmx::Matrix3 &, Vector3 &);
void MakeEulerScale(const Hmx::Matrix3 &, Vector3 &, Vector3 &);
void MakeRotMatrix(const Vector3 &, Hmx::Matrix3 &, bool);
void MakeRotMatrix(const Vector3 &, const Vector3 &, Hmx::Matrix3 &);
void MakeRotMatrix(const Hmx::Quat &, Hmx::Matrix3 &);
void RotateAboutX(const Hmx::Matrix3 &, float, Hmx::Matrix3 &);
void RotateAboutZ(const Hmx::Matrix3 &, float, Hmx::Matrix3 &);

inline void MakeRotMatrixX(float angle, Hmx::Matrix3 &m) {
    float c = Cosine(angle);
    float s = Sine(angle);
    m.Set(1.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f, -s, c);
}

inline void MakeRotMatrixY(float angle, Hmx::Matrix3 &m) {
    float c = Cosine(angle);
    float s = Sine(angle);
    m.Set(c, 0.0f, -s, 0.0f, 1.0f, 0.0f, s, 0.0f, c);
}

inline void MakeRotMatrixZ(float angle, Hmx::Matrix3 &m) {
    float c = Cosine(angle);
    float s = Sine(angle);
    m.Set(c, s, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 1.0f);
}

void MakeEuler(const Hmx::Quat &, Vector3 &);
void MakeRotQuat(const Vector3 &, const Vector3 &, Hmx::Quat &);
void MakeRotQuatUnitX(const Vector3 &, Hmx::Quat &);
void Multiply(const Vector3 &, const Hmx::Quat &, Vector3 &);

void Normalize(const Hmx::Quat &, Hmx::Quat &);
void FastInterp(const Hmx::Quat &, const Hmx::Quat &, float, Hmx::Quat &);
void IdentityInterp(const Hmx::Quat &, float, Hmx::Quat &);
void Nlerp(const Hmx::Quat &, const Hmx::Quat &, float, Hmx::Quat &);
void Interp(const Hmx::Quat &, const Hmx::Quat &, float, Hmx::Quat &);
void Interp(const Hmx::Matrix3 &, const Hmx::Matrix3 &, float, Hmx::Matrix3 &);

inline void RotateAboutZ(const Vector3 &vin, float f2, Vector3 &vres) {
    float c = Cosine(f2);
    float s = Sine(f2);
    vres.Set(vin.x * c - vin.y * s, vin.x * s + vin.y * c, vin.z);
}
