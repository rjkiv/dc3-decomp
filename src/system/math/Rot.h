#pragma once
#include "math/Mtx.h"
#include "math/Trig.h"
#include "math/Vec.h"

namespace Hmx {
    class Quat {
    public:
        Quat() {}
        Quat(float xIn, float yIn, float zIn, float wIn)
            : x(xIn), y(yIn), z(zIn), w(wIn) {}
        Quat(const Matrix3 &m) { Set(m); }
        Quat(const Vector3 &v) { Set(v); }
        Quat(const Vector3 &, float);

        void Reset() {
            x = y = z = 0.0f;
            w = 1.0f;
        }
        void Zero() { w = x = y = z = 0.0f; }
        void Set(const Matrix3 &);
        void Set(const Vector3 &);
        void Set(const Vector3 &, float);
        void Set(float xIn, float yIn, float zIn, float wIn) {
            x = xIn;
            y = yIn;
            z = zIn;
            w = wIn;
        }

        float operator*(const Quat &q) const {
            return x * q.x + y * q.y + z * q.z + w * q.w;
        }

        bool operator!=(const Quat &q) const {
            return x != q.x || y != q.y || z != q.z || w != q.w;
        }

        const float &operator[](int i) const { return *(&x + i); }
        float &operator[](int i) { return *(&x + i); }

        float x; // 0x0
        float y; // 0x4
        float z; // 0x8
        float w; // 0xc
    };
}

inline BinStream &operator<<(BinStream &bs, const Hmx::Quat &q) {
    bs << q.x << q.y << q.z << q.w;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::Quat &q) {
    bs >> q.x >> q.y >> q.z >> q.w;
    return bs;
}

class ShortQuat {
public:
    void Set(const Hmx::Quat &);
    void ToQuat(Hmx::Quat &q) const {
        q.Set(
            x * 0.000030518509f,
            y * 0.000030518509f,
            z * 0.000030518509f,
            w * 0.000030518509f
        );
    }

    short x;
    short y;
    short z;
    short w;
};

class ByteQuat {
public:
    void Set(const Hmx::Quat &);
    void ToQuat(Hmx::Quat &q) const {
        q.Set(x * 0.007874016f, y * 0.007874016f, z * 0.007874016f, w * 0.007874016f);
    }

    char x;
    char y;
    char z;
    char w;
};

class QuatXfm {
public:
    QuatXfm() {}
    QuatXfm(const Transform &);

    Vector3 v;
    Hmx::Quat q;
};

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

inline void Multiply(const Hmx::Quat &q1, const Hmx::Quat &q2, Hmx::Quat &qres) {
    qres.Set(
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
        q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
    );
}

inline void Negate(const Hmx::Quat &in, Hmx::Quat &out) {
    out.Set(-in.x, -in.y, -in.z, in.w);
}

inline void NormalizeTo(const Hmx::Quat &qin, Hmx::Quat &qout) {
    if (qin * qout < 0) {
        qout.x = -qout.x;
        qout.y = -qout.y;
        qout.z = -qout.z;
        qout.w = -qout.w;
    }
}

void ScaleAddEq(Hmx::Quat &, const Hmx::Quat &, float);

inline void
FasterInterp(const Hmx::Quat &q1, const Hmx::Quat &q2, float f, Hmx::Quat &qres) {
    qres.x = Interp(q1.x, q2.x, f);
    qres.y = Interp(q1.y, q2.y, f);
    qres.z = Interp(q1.z, q2.z, f);
    qres.w = Interp(q1.w, q2.w, f);
}
