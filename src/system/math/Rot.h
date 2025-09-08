#pragma once
#include "math/Mtx.h"
#include "math/Vec.h"

#define PI 3.1415927f
#define RAD2DEG 57.29578f
#define DEG2RAD 0.01745329238474369049f

TextStream &operator<<(TextStream &ts, const Hmx::Quat &v);
TextStream &operator<<(TextStream &ts, const Vector3 &v);
TextStream &operator<<(TextStream &ts, const Vector2 &v);
TextStream &operator<<(TextStream &ts, const Hmx::Matrix3 &m);
TextStream &operator<<(TextStream &ts, const Transform &t);

float GetXAngle(const Hmx::Matrix3 &);
float GetYAngle(const Hmx::Matrix3 &);
float GetZAngle(const Hmx::Matrix3 &);

void MakeEuler(const Hmx::Matrix3 &, Vector3 &);
void MakeScale(const Hmx::Matrix3 &, Vector3 &);
void MakeEulerScale(const Hmx::Matrix3 &, Vector3 &, Vector3 &);
void MakeRotMatrix(const Vector3 &, const Hmx::Matrix3 &, bool);
void MakeRotMatrix(const Vector3 &, const Vector3 &, Hmx::Matrix3 &);
void RotateAboutX(const Hmx::Matrix3 &, float, Hmx::Matrix3 &);
void RotateAboutZ(const Hmx::Matrix3 &, float, Hmx::Matrix3 &);
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

void RotateAboutZ(const Vector3 &, float, Vector3 &);
