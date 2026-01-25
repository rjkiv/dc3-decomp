#include "Rot.h"
#include "math/Mtx.h"
#include "math/Trig.h"
#include "math/Vec.h"
#include "os/Debug.h"
#include <cmath>

TextStream &operator<<(TextStream &ts, const Hmx::Quat &v) {
    ts << "(x:" << v.x << " y:" << v.y << " z:" << v.z << " w:" << v.w << ")";
    return ts;
}

TextStream &operator<<(TextStream &ts, const Vector3 &v) {
    ts << "(x:" << v.x << " y:" << v.y << " z:" << v.z << ")";
    return ts;
}

TextStream &operator<<(TextStream &ts, const Vector2 &v) {
    ts << "(x:" << v.x << " y:" << v.y << ")";
    return ts;
}

TextStream &operator<<(TextStream &ts, const Hmx::Matrix3 &m) {
    ts << "\n\t" << m.x << "\n\t" << m.y << "\n\t" << m.z;
    return ts;
}

TextStream &operator<<(TextStream &ts, const Transform &t) {
    ts << t.m << "\n\t" << t.v;
    return ts;
}

Hmx::Matrix4 &Hmx::Matrix4::Zero() {
    memset(this, 0, sizeof(*this));
    return *this;
}

void Hmx::Quat::Set(const Vector3 &v, float f) {
    float scale = Sine(f / 2);
    w = Cosine(f / 2);
    x = v.x * scale;
    y = v.y * scale;
    z = v.z * scale;
}

float GetXAngle(const Hmx::Matrix3 &m) { return atan2(m.y.z, m.y.y); }
float GetYAngle(const Hmx::Matrix3 &m) { return atan2(-m.x.z, m.z.z); }
float GetZAngle(const Hmx::Matrix3 &m) {
    float res = atan2(m.y.x, m.y.y);
    return -res;
}

void MakeEuler(const Hmx::Matrix3 &m, Vector3 &v) {
    if (fabsf(m.y.z) > 0.99999988f) {
        v.x = m.y.z > 0 ? PI / 2 : -PI / 2;
        v.z = std::atan2(m.x.y, m.x.x);
        v.y = 0;
    } else {
        v.z = std::atan2(-m.y.x, m.y.y);
        v.x = std::asin(m.y.z);
        v.y = GetYAngle(m);
    }
}

void MakeScale(const Hmx::Matrix3 &m, Vector3 &v) {
    float zlen = Length(m.z);
    Vector3 cross;
    Cross(m.x, m.y, cross);
    if (Dot(cross, m.z) <= 0) {
        zlen = -zlen;
    }
    v.Set(Length(m.x), Length(m.y), zlen);
}

void MakeEulerScale(const Hmx::Matrix3 &m1, Vector3 &v2, Vector3 &v3) {
    MakeScale(m1, v3);
    Hmx::Matrix3 m38;
    m38.z.z = 0;
    m38.x.z = v3.x ? 1.0f / v3.x : m38.z.z;
    Scale(m1.x, m38.x.z, m38.x);
    if (v3.y != 0) {
        m38.y.z = 1.0f / v3.y;
    }
    Scale(m1.y, m38.y.z, m38.y);
    if (v3.z != 0) {
        m38.z.z = 1.0f / v3.z;
    }
    Scale(m1.z, m38.z.z, m38.z);
    MakeEuler(m38, v2);
}

void Normalize(const Hmx::Quat &qin, Hmx::Quat &qout) {
    float res = qin * qin;
    if (res == 0) {
        MILO_NOTIFY_ONCE("trying to normalize zero quat, probable error");
        qout.Set(0, 0, 1, 0);
    } else {
        res = 1 / sqrtf(res);
        qout.Set(qin.x * res, qin.y * res, qin.z * res, qin.w * res);
    }
}

void Interp(const Hmx::Quat &q1, const Hmx::Quat &q2, float r, Hmx::Quat &qres) {
    Nlerp(q1, q2, r, qres);
}

void Interp(const Hmx::Matrix3 &m1, const Hmx::Matrix3 &m2, float r, Hmx::Matrix3 &res) {
    Hmx::Quat q40(m1);
    Hmx::Quat q50(m2);
    Hmx::Quat q60;
    Nlerp(q40, q50, r, q60);
    MakeRotMatrix(q60, res);
}

void MakeEuler(const Hmx::Quat &q, Vector3 &v) {
    Hmx::Matrix3 m;
    MakeRotMatrix(q, m);
    MakeEuler(m, v);
}

void MakeRotMatrix(const Hmx::Quat &q, Hmx::Matrix3 &mtx) {
    float qxx = q.x * q.x * 2.0f;
    float qxy = q.x * q.y * 2.0f;
    float qxz = q.x * q.z * 2.0f;
    float qxw = q.x * q.w * 2.0f;
    float qyy = q.y * q.y * 2.0f;
    float qyz = q.y * q.z * 2.0f;
    float qyw = q.y * q.w * 2.0f;
    float qzz = q.z * q.z * 2.0f;
    float qzw = q.z * q.w * 2.0f;
    mtx.x.x = (1.0f - qyy) - qzz;
    mtx.x.y = qzw + qxy;
    mtx.x.z = qxz - qyw;
    mtx.y.x = qxy - qzw;
    mtx.y.y = (1.0f - qzz) - qxx;
    mtx.y.z = qyz + qxw;
    mtx.z.x = qyw + qxz;
    mtx.z.y = qyz - qxw;
    mtx.z.z = (1.0f - qxx) - qyy;
}

void MakeRotMatrix(const Vector3 &v, Hmx::Matrix3 &mtx, bool lookup) {
    float xsin, xcos, ysin, ycos, zsin, zcos;
    if (lookup) {
        zsin = Sine(v.z);
        zcos = Cosine(v.z);
        ysin = Sine(v.y);
        ycos = Cosine(v.y);
        xsin = Sine(v.x);
        xcos = Cosine(v.x);
    } else {
        zsin = sin(v.z);
        zcos = cos(v.z);
        ysin = sin(v.y);
        ycos = cos(v.y);
        xsin = sin(v.x);
        xcos = cos(v.x);
    }

    mtx.y.z = xsin;
    mtx.y.y = xcos * zcos;
    mtx.x.x = -(xsin * ysin * zsin - ycos * zcos);
    mtx.z.z = xcos * ycos;
    mtx.x.y = xsin * ysin * zcos + ycos * zsin;
    mtx.x.z = -(xcos * ysin);
    mtx.y.x = -(xcos * zsin);
    mtx.z.y = -(ycos * zcos * xsin + ysin * zsin);
    mtx.z.x = ycos * zsin * xsin + ysin * zcos;
}

void MakeRotMatrix(const Vector3 &v1, const Vector3 &v2, Hmx::Matrix3 &mtx) {
    mtx.y = v1;
    Normalize(mtx.y, mtx.y);
    Cross(mtx.y, v2, mtx.x);
    Normalize(mtx.x, mtx.x);
    Cross(mtx.x, mtx.y, mtx.z);
}

void RotateAboutX(const Hmx::Matrix3 &min, float f, Hmx::Matrix3 &mout) {
    float fcos = Cosine(f);
    float fsin = Sine(f);
    mout.Set(
        min.x.x,
        min.x.y * fcos - min.x.z * fsin,
        min.x.y * fsin + min.x.z * fcos,
        min.y.x,
        min.y.y * fcos - min.y.z * fsin,
        min.y.y * fsin + min.y.z * fcos,
        min.z.x,
        min.z.y * fcos - min.z.z * fsin,
        min.z.y * fsin + min.z.z * fcos
    );
}

void RotateAboutZ(const Hmx::Matrix3 &min, float f, Hmx::Matrix3 &mout) {
    float fcos = Cosine(f);
    float fsin = Sine(f);
    mout.Set(
        min.x.x * fcos - min.x.y * fsin,
        min.x.x * fsin + min.x.y * fcos,
        min.x.z,
        min.y.x * fcos - min.y.y * fsin,
        min.y.x * fsin + min.y.y * fcos,
        min.y.z,
        min.z.x * fcos - min.z.y * fsin,
        min.z.x * fsin + min.z.y * fcos,
        min.z.z
    );
}

void MakeRotMatrixX(float angle, Hmx::Matrix3 &m) {
    float c = Cosine(angle);
    float s = Sine(angle);
    m.Set(1.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f, -s, c);
}

void MakeRotMatrixY(float angle, Hmx::Matrix3 &m) {
    float c = Cosine(angle);
    float s = Sine(angle);
    m.Set(c, 0.0f, -s, 0.0f, 1.0f, 0.0f, s, 0.0f, c);
}

void MakeRotMatrixZ(float angle, Hmx::Matrix3 &m) {
    float c = Cosine(angle);
    float s = Sine(angle);
    m.Set(c, -s, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 1.0f);
}

void MakeRotQuat(const Vector3 &v1, const Vector3 &v2, Hmx::Quat &q) {
    Vector3 vec;
    Cross(v1, v2, vec);
    float sq = std::sqrt(LengthSquared(v1) * LengthSquared(v2));
    float sq2 = std::sqrt(0.5f + ((Dot(v1, v2) / 2.0f) / sq));
    if (sq2 > 1e-7f) {
        float f1 = 0.5f / (sq * sq2);
        q.Set(vec.x * f1, vec.y * f1, vec.z * f1, sq2);
    } else {
        q.Set(0, 0, 1, 0);
    }
}

void MakeRotQuatUnitX(const Vector3 &vec, Hmx::Quat &q) {
    float sq = std::sqrt(vec.x / 2.0f + 0.5f);
    if (sq > 1e-7f) {
        q.Set(0, vec.z * (0.5f / sq), -vec.y * (0.5f / sq), sq);
    } else {
        q.Set(0, 0, 1, 0);
    }
}

void Multiply(const Vector3 &vin, const Hmx::Quat &q, Vector3 &vout) {
    vout.Set(
        (vin.z * (q.w * q.y + q.x * q.z) + vin.x * (-q.y * q.y + -q.z * q.z)
         + vin.y * (q.x * q.y - q.w * q.z))
                * 2.0f
            + vin.x,
        (vin.z * (q.y * q.z - q.w * q.x) + vin.x * (q.w * q.z + q.x * q.y)
         + vin.y * (-q.x * q.x + -q.z * q.z))
                * 2.0f
            + vin.y,
        (vin.z * (-q.x * q.x + -q.y * q.y) + vin.x * (q.x * q.z - q.w * q.y)
         + vin.y * (q.w * q.x + q.y * q.z))
                * 2.0f
            + vin.z
    );
}

void FastInterp(const Hmx::Quat &q1, const Hmx::Quat &q2, float f, Hmx::Quat &qout) {
    if (f == 0) {
        qout = q1;
    } else if (f == 1) {
        qout = q2;
    } else {
        if (q1 * q2 < 0) {
            qout.x = -(f * (q2.x + q1.x) - q1.x);
            qout.y = -(f * (q2.y + q1.y) - q1.y);
            qout.z = -(f * (q2.z + q1.z) - q1.z);
            qout.w = -(f * (q2.w + q1.w) - q1.w);
        } else {
            qout.x = f * (q2.x - q1.x) + q1.x;
            qout.y = f * (q2.y - q1.y) + q1.y;
            qout.z = f * (q2.z - q1.z) + q1.z;
            qout.w = f * (q2.w - q1.w) + q1.w;
        }
        Normalize(qout, qout);
    }
}

void IdentityInterp(const Hmx::Quat &qin, float f, Hmx::Quat &qout) {
    if (f == 0) {
        qout = qin;
    } else if (f == 1) {
        qout.Set(0, 0, 0, 1);
    } else {
        float diff = 1.0f - f;
        qout.x = qin.x * diff;
        qout.y = qin.y * diff;
        qout.z = qin.z * diff;
        if (qin.w < 0) {
            qout.w = qin.w * diff - f;
        } else {
            qout.w = qin.w * diff + f;
        }
        Normalize(qout, qout);
    }
}

void Nlerp(const Hmx::Quat &q1, const Hmx::Quat &q2, float f, Hmx::Quat &qout) {
    if (f == 0) {
        qout = q1;
    } else if (f == 1) {
        qout = q2;
    } else {
        if (q1 * q2 < 0) {
            qout.x = -((q1.x + q2.x) * f - q1.x);
            qout.y = -((q1.y + q2.y) * f - q1.y);
            qout.z = -((q1.z + q2.z) * f - q1.z);
            qout.w = -((q1.w + q2.w) * f - q1.w);
        } else {
            qout.x = -((q1.x - q2.x) * f - q1.x);
            qout.y = -((q1.y - q2.y) * f - q1.y);
            qout.z = -((q1.z - q2.z) * f - q1.z);
            qout.w = -((q1.w - q2.w) * f - q1.w);
        }
        Normalize(qout, qout);
    }
}
