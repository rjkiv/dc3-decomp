#pragma once
#include "Mtx.h"
#include "math/Utl.h"

#pragma region Hmx::Matrix3

inline BinStream &operator<<(BinStream &bs, const Hmx::Matrix3 &mtx) {
    bs << mtx.x << mtx.y << mtx.z;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::Matrix3 &mtx) {
    bs >> mtx.x >> mtx.y >> mtx.z;
    return bs;
}

void Interp(const Hmx::Matrix3 &, const Hmx::Matrix3 &, float, Hmx::Matrix3 &);
void Multiply(const Hmx::Matrix3 &, const Hmx::Matrix3 &, Hmx::Matrix3 &);

inline void Normalize(const Hmx::Matrix3 &in, Hmx::Matrix3 &out) {
    Normalize(in.y, out.y);
    Cross(out.y, in.z, out.x);
    Normalize(out.x, out.x);
    Cross(out.x, out.y, out.z);
}

inline void NormalizeAboutX(Hmx::Matrix3 &mtx) {
    Cross(mtx.x, mtx.y, mtx.z);
    Normalize(mtx.z, mtx.z);
    Cross(mtx.z, mtx.x, mtx.y);
}

inline void NormalizeAboutY(Hmx::Matrix3 &mtx) {
    Cross(mtx.x, mtx.y, mtx.z);
    Normalize(mtx.z, mtx.z);
    Cross(mtx.y, mtx.z, mtx.x);
}

inline void Multiply(const Hmx::Matrix3 &m, const Vector3 &v, Vector3 &vout) {
    vout.Set(
        m.x.x * v.x + m.y.x * v.y + m.z.x * v.z,
        m.x.y * v.x + m.y.y * v.y + m.z.y * v.z,
        m.x.z * v.x + m.y.z * v.y + m.z.z * v.z
    );
}

inline void Multiply(const Vector3 &v, const Hmx::Matrix3 &m, Vector3 &vout) {
    vout.Set(
        m.x.x * v.x + m.y.x * v.y + m.z.x * v.z,
        m.x.y * v.x + m.y.y * v.y + m.z.y * v.z,
        m.x.z * v.x + m.y.z * v.y + m.z.z * v.z
    );
}

inline void Transpose(const Hmx::Matrix3 &in, Hmx::Matrix3 &out) {
    out.Set(in.x.x, in.y.x, in.z.x, in.x.y, in.y.y, in.z.y, in.x.z, in.y.z, in.z.z);
}

// so Scale with Matrix first, then Vector, calls Scale(Vector3,Vector3,Vector3)...
inline void Scale(const Hmx::Matrix3 &mtx, const Vector3 &vec, Hmx::Matrix3 &res) {
    Scale(mtx.x, vec, res.x);
    Scale(mtx.y, vec, res.y);
    Scale(mtx.z, vec, res.z);
}

// but Scale with Vector first, then Matrix, calls Scale(Vector3,float,Vector3)
// ok HMX that's cool and totally won't trip somebody up in the future
inline void Scale(const Vector3 &vec, const Hmx::Matrix3 &mtx, Hmx::Matrix3 &res) {
    Scale(mtx.x, vec.x, res.x);
    Scale(mtx.y, vec.y, res.y);
    Scale(mtx.z, vec.z, res.z);
}

inline void ScaleAddEq(Hmx::Matrix3 &dst, const Hmx::Matrix3 &src, float scalar) {
    ScaleAddEq(dst.x, src.x, scalar);
    ScaleAddEq(dst.y, src.y, scalar);
    ScaleAddEq(dst.z, src.z, scalar);
}

#pragma endregion
#pragma region Transform

inline void Transform::LookAt(const Vector3 &v1, const Vector3 &v2) {
    Subtract(v1, v, m.y);
    m.z = v2;
    Normalize(m, m);
}

inline BinStream &operator<<(BinStream &bs, const Transform &tf) {
    bs << tf.m << tf.v;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Transform &tf) {
    bs >> tf.m >> tf.v;
    return bs;
}

inline BinStream &operator>>(BinStreamRev &d, Transform &tf) { return d.stream >> tf; }

void Multiply(const Transform &, const Hmx::Matrix3 &, Transform &);

inline void MultiplyTranspose(const Vector3 &v, const Transform &t, Vector3 &out) {
    Subtract(v, t.v, out);
    out.Set(Dot(out, t.m.x), Dot(out, t.m.y), Dot(out, t.m.z));
}

inline void Multiply(const Vector3 &v, const Transform &t, Vector3 &out) {
    if (&t.v != &out) {
        Multiply(t.m, v, out);
        Add(out, t.v, out);
    } else {
        Vector3 tmp;
        Multiply(t.m, v, tmp);
        Add(tmp, t.v, out);
    }
}

inline void Invert(const Transform &in, Transform &out) {
    Vector3 inV;
    Negate(in.v, inV);
    Invert(in.m, out.m);
    Multiply(inV, out.m, out.v);
}

inline void FastInvert(const Transform &in, Transform &out) {
    Vector3 inV;
    Negate(in.v, inV);
    FastInvert(in.m, out.m);
    Multiply(inV, out.m, out.v);
}

inline void Transpose(const Transform &in, Transform &out) {
    Transpose(in.m, out.m);
    Vector3 inV;
    Negate(in.v, inV);
    out.v.Set(
        out.m.x.x * inV.x + out.m.y.x * inV.y + out.m.z.x * inV.z,
        out.m.x.y * inV.x + out.m.y.y * inV.y + out.m.z.y * inV.z,
        out.m.x.z * inV.x + out.m.y.z * inV.y + out.m.z.z * inV.z
    );
}

inline void MultiplyInverse(const Transform &t1, const Transform &t2, Transform &tres) {
    Hmx::Matrix3 m50;
    Invert(t2.m, m50);
    Multiply(t1.m, m50, tres.m);
    Vector3 diff;
    Subtract(t1.v, t2.v, diff);
    Multiply(diff, m50, tres.v);
}

inline void ScaleAddEq(Transform &dst, const Transform &src, float scalar) {
    ScaleAddEq(dst.m, src.m, scalar);
    ScaleAddEq(dst.v, src.v, scalar);
}

#pragma endregion
#pragma region Hmx::Matrix4

inline Hmx::Matrix4::Matrix4(const Transform &xfm) {
    m[0].x = xfm.m.x.x;
    m[0].y = xfm.m.x.y;
    m[0].z = xfm.m.x.z;
    m[0].w = 0;
    m[1].x = xfm.m.y.x;
    m[1].y = xfm.m.y.y;
    m[1].z = xfm.m.y.z;
    m[1].w = 0;
    m[2].x = xfm.m.z.x;
    m[2].y = xfm.m.z.y;
    m[2].z = xfm.m.z.z;
    m[2].w = 0;
    m[3].x = xfm.v.x;
    m[3].y = xfm.v.y;
    m[3].z = xfm.v.z;
    m[3].w = 1;
}

void Multiply(const Vector4 &, const Hmx::Matrix4 &, Vector4 &);
void Transpose(const Hmx::Matrix4 &, Hmx::Matrix4 &);

#pragma endregion
#pragma region Hmx::Quat

inline BinStream &operator<<(BinStream &bs, const Hmx::Quat &q) {
    bs << q.x << q.y << q.z << q.w;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::Quat &q) {
    bs >> q.x >> q.y >> q.z >> q.w;
    return bs;
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

#pragma endregion
