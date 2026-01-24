#pragma once
#include "math/Sphere.h"
#include "math/Vec.h"
#include "math/Trig.h"
#include "utl/BinStream.h"

class Transform;

namespace Hmx {
    class Matrix2 {
    private:
        static Matrix2 sID;

    public:
        Matrix2(const Vector2 &v1, const Vector2 &v2) : x(v1), y(v2) {}
        Vector2 x;
        Vector2 y;
    };

    class Matrix3 {
    private:
        static Matrix3 sID;

    public:
        Vector3 x;
        Vector3 y;
        Vector3 z;

        // all of these are weak
        Matrix3() {}

        // Matrix3(const Matrix3 &mtx) {
        //     x = mtx.x;
        //     y = mtx.y;
        //     z = mtx.z;
        // }

        Matrix3(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3)
            : x(v1), y(v2), z(v3) {}

        // clang-format off
        Matrix3(
            float f1, float f2, float f3,
            float f4, float f5, float f6,
            float f7, float f8, float f9
        )
            : x(f1, f2, f3), y(f4, f5, f6), z(f7, f8, f9) {}

        void Set(
            float f1, float f2, float f3,
            float f4, float f5, float f6,
            float f7, float f8, float f9
        ) {
            x.Set(f1, f2, f3);
            y.Set(f4, f5, f6);
            z.Set(f7, f8, f9);
        }
        // clang-format on
        void Set(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
            x = v1;
            y = v2;
            z = v3;
        }
        void Zero() {
            x.Zero();
            y.Zero();
            z.Zero();
        }
        void RotateAboutZ(float angle) {
            float c = Cosine(angle);
            float s = Sine(angle);
            Set(c, -s, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 1.0f);
        }
        void RotateAboutY(float angle) {
            float c = Cosine(angle);
            float s = Sine(angle);
            Set(c, 0.0f, -s, 0.0f, 1.0f, 0.0f, s, 0.0f, c);
        }
        void RotateAboutX(float angle) {
            float c = Cosine(angle);
            float s = Sine(angle);
            Set(1.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f, -s, c);
        }
        void Identity() {
            x.Set(1.0f, 0.0f, 0.0f);
            y.Set(0.0f, 1.0f, 0.0f);
            z.Set(0.0f, 0.0f, 1.0f);
        }
        Matrix3 &operator=(const Matrix3 &mtx) {
            memcpy(this, &mtx, sizeof(*this));
            return *this;
        }
        Vector3 &operator[](int i) { return *(&x + i); }

        bool operator==(const Matrix3 &mtx) const {
            return x == mtx.x && y == mtx.y && z == mtx.z;
        }

        bool operator!=(const Matrix3 &mtx) const {
            return x != mtx.x || y != mtx.y || z != mtx.z;
        }

        static const Hmx::Matrix3 &GetIdentity() { return sID; }
    };

    class Matrix4 {
    private:
        static Matrix4 sID;

    public:
        Matrix4() {}
        Matrix4(const Transform &);
        Matrix4(const Vector4 &v1, const Vector4 &v2, const Vector4 &v3, const Vector4 &v4)
            : x(v1), y(v2), z(v3), w(v4) {}

        Matrix4 &Zero();
        Matrix4 &operator=(const Matrix4 &mtx) {
            memcpy(this, &mtx, sizeof(*this));
            return *this;
        }
        static const Hmx::Matrix4 &ID() { return sID; }

        Vector4 x;
        Vector4 y;
        Vector4 z;
        Vector4 w;
    };

    Hmx::Matrix4 operator*(const Transform &, const Hmx::Matrix4 &);

    class Quat {
    public:
        Quat() {}
        Quat(float f1, float f2, float f3, float f4) : x(f1), y(f2), z(f3), w(f4) {}
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
        void Set(float f1, float f2, float f3, float f4) {
            x = f1;
            y = f2;
            z = f3;
            w = f4;
        }

        float operator*(const Quat &q) const {
            return x * q.x + y * q.y + z * q.z + w * q.w;
        }

        bool operator!=(const Quat &q) const {
            return x != q.x || y != q.y || z != q.z || w != q.w;
        }

        const float &operator[](int i) const { return *(&x + i); }
        float &operator[](int i) { return *(&x + i); }

        float x;
        float y;
        float z;
        float w;
    };
}

inline BinStream &operator<<(BinStream &bs, const Hmx::Matrix3 &mtx) {
    bs << mtx.x << mtx.y << mtx.z;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::Matrix3 &mtx) {
    bs >> mtx.x >> mtx.y >> mtx.z;
    return bs;
}

inline BinStream &operator<<(BinStream &bs, const Hmx::Quat &q) {
    bs << q.x << q.y << q.z << q.w;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::Quat &q) {
    bs >> q.x >> q.y >> q.z >> q.w;
    return bs;
}

class Transform {
private:
    static Transform sID;

public:
    class Hmx::Matrix3 m;
    class Vector3 v;

    // all of these are weak
    Transform() {}

    Transform(const Hmx::Matrix3 &mtx, const Vector3 &vec) : m(mtx), v(vec) {}

    // Transform(const Transform &tf);
    Transform &operator=(const Transform &tf) {
        memcpy(this, &tf, sizeof(*this));
        return *this;
    }

    void Reset() {
        m.Identity();
        v.Zero();
    }

    void Set(const Hmx::Matrix3 &mtx, const Vector3 &vec) {
        m = mtx;
        v = vec;
    }

    void LookAt(const Vector3 &, const Vector3 &);
    void Zero() {
        m.Zero();
        v.Zero();
    }

    bool operator==(const Transform &tf) const { return m == tf.m && v == tf.v; }
    bool operator!=(const Transform &tf) const { return m != tf.m || v != tf.v; }

    static const Transform &IDXfm() { return sID; }
};

inline BinStream &operator<<(BinStream &bs, const Transform &tf) {
    bs << tf.m << tf.v;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Transform &tf) {
    bs >> tf.m >> tf.v;
    return bs;
}

inline BinStream &operator>>(BinStreamRev &bs, Transform &tf) { return bs.stream >> tf; }

class QuatXfm {
public:
    Vector3 v;
    Hmx::Quat q;
};

class Plane {
public:
    Plane() {}

    void Set(const Vector3 &, const Vector3 &, const Vector3 &);
    void Set(float f1, float f2, float f3, float f4) {
        a = f1;
        b = f2;
        c = f3;
        d = f4;
    }
    float Dot(const Vector3 &vec) const { return a * vec.x + b * vec.y + c * vec.z + d; }
    Vector3 On() const {
        Vector3 ret;
        float scalar = -d / (a * a + b * b + c * c);
        ret.Set(a * scalar, b * scalar, c * scalar);
        return ret;
    }

    float a, b, c, d;
};

void Normalize(const Plane &, Plane &);

inline BinStream &operator<<(BinStream &bs, const Plane &p) {
    bs << p.a << p.b << p.c << p.d;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Plane &p) {
    bs >> p.a >> p.b >> p.c >> p.d;
    return bs;
}

class Frustum {
    // total size: 0x60
public:
    void Set(float, float, float, float);

    class Plane front; // offset 0x0, size 0x10
    class Plane back; // offset 0x10, size 0x10
    class Plane left; // offset 0x20, size 0x10
    class Plane right; // offset 0x30, size 0x10
    class Plane top; // offset 0x40, size 0x10
    class Plane bottom; // offset 0x50, size 0x10
};

// defined in mtx.cpp
float Det(const Hmx::Matrix3 &m);
void Invert(const Hmx::Matrix3 &, Hmx::Matrix3 &);
void FastInvert(const Hmx::Matrix3 &, Hmx::Matrix3 &);
void Multiply(const Transform &, const Transform &, Transform &);
float Det(const Hmx::Matrix4 &);
void Invert(const Hmx::Matrix4 &, Hmx::Matrix4 &);

bool operator>(const Sphere &, const Frustum &);

inline void Normalize(const Hmx::Matrix3 &in, Hmx::Matrix3 &out) {
    Normalize(in.y, out.y);
    out.x.Set(
        out.y.y * in.z.z - out.y.z * in.z.y,
        out.y.z * in.z.x - out.y.x * in.z.z,
        out.y.x * in.z.y - out.y.y * in.z.x
    );
    Normalize(out.x, out.x);
    out.z.Set(
        out.y.z * out.x.y - out.y.y * out.x.z,
        out.y.x * out.x.z - out.y.z * out.x.x,
        out.y.y * out.x.x - out.y.x * out.x.y
    );
}

void Multiply(const Hmx::Matrix3 &, const Hmx::Matrix3 &, Hmx::Matrix3 &);
void Multiply(const Vector3 &, const Transform &, Vector3 &);

inline void MultiplyTranspose(const Vector3 &v, const Transform &t, Vector3 &out) {
    Subtract(v, t.v, out);
    out.Set(Dot(out, t.m.x), Dot(out, t.m.y), Dot(out, t.m.z));
}

inline void Multiply(const Vector3 &v, const Transform &t, Vector3 &out) {
    if (&t.v != &out) {
        out.Set(
            t.m.x.x * v.x + t.m.y.x * v.y + t.m.z.x * v.z,
            t.m.x.y * v.x + t.m.y.y * v.y + t.m.z.y * v.z,
            t.m.x.z * v.x + t.m.y.z * v.y + t.m.z.z * v.z
        );
        Add(out, t.v, out);
    } else {
        out.Set(
            t.m.x.x * v.x + t.m.y.x * v.y + t.m.z.x * v.z + t.v.x,
            t.m.x.y * v.x + t.m.y.y * v.y + t.m.z.y * v.z + t.v.y,
            t.m.x.z * v.x + t.m.y.z * v.y + t.m.z.z * v.z + t.v.z
        );
    }
}

void Multiply(const Plane &, const Transform &, Plane &);

inline void Multiply(const Hmx::Quat &q1, const Hmx::Quat &q2, Hmx::Quat &qres) {
    qres.Set(
        -(q1.z * q2.y - (q1.y * q2.z + q1.w * q2.x + q1.x * q2.w)),
        -(q1.x * q2.z - (q1.z * q2.x + q1.w * q2.y + q1.y * q2.w)),
        -(q1.y * q2.x - (q1.x * q2.y + q1.w * q2.z + q1.z * q2.w)),
        -(q1.z * q2.z - -(q1.y * q2.y - (q1.w * q2.w - q1.x * q2.x)))
    );
}

inline void Multiply(const Vector3 &v, const Hmx::Matrix3 &m, Vector3 &vout) {
    vout.Set(
        m.x.x * v.x + m.y.x * v.y + m.z.x * v.z,
        m.x.y * v.x + m.y.y * v.y + m.z.y * v.z,
        m.x.z * v.x + m.y.z * v.y + m.z.z * v.z
    );
}

inline void Invert(const Transform &in, Transform &out) {
    Vector3 inV;
    Negate(in.v, inV);
    Invert(in.m, out.m);
    out.v.Set(
        out.m.x.x * inV.x + out.m.y.x * inV.y + out.m.z.x * inV.z,
        out.m.x.y * inV.x + out.m.y.y * inV.y + out.m.z.y * inV.z,
        out.m.x.z * inV.x + out.m.y.z * inV.y + out.m.z.z * inV.z
    );
}

inline void FastInvert(const Transform &in, Transform &out) {
    Vector3 inV;
    Negate(in.v, inV);
    FastInvert(in.m, out.m);
    out.v.Set(
        out.m.x.x * inV.x + out.m.y.x * inV.y + out.m.z.x * inV.z,
        out.m.x.y * inV.x + out.m.y.y * inV.y + out.m.z.y * inV.z,
        out.m.x.z * inV.x + out.m.y.z * inV.y + out.m.z.z * inV.z
    );
}

void Transpose(const Hmx::Matrix4 &, Hmx::Matrix4 &);

inline void Multiply(const Frustum &fin, const Transform &tf, Frustum &fout) {
    Multiply(fin.front, tf, fout.front);
    Multiply(fin.back, tf, fout.back);
    Multiply(fin.left, tf, fout.left);
    Multiply(fin.right, tf, fout.right);
    Multiply(fin.top, tf, fout.top);
    Multiply(fin.bottom, tf, fout.bottom);
}

inline void Transpose(const Transform &in, Transform &out) {
    out.m.Set(
        in.m.x.x,
        in.m.y.x,
        in.m.z.x,
        in.m.x.y,
        in.m.y.y,
        in.m.z.y,
        in.m.x.z,
        in.m.y.z,
        in.m.z.z
    );
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

inline void Scale(const Hmx::Matrix3 &mtx, const Vector3 &vec, Hmx::Matrix3 &res) {
    Scale(mtx.x, vec, res.x);
    Scale(mtx.y, vec, res.y);
    Scale(mtx.z, vec, res.z);
}

inline void Scale(const Vector3 &vec, const Hmx::Matrix3 &mtx, Hmx::Matrix3 &res) {
    Scale(mtx.x, vec.x, res.x);
    Scale(mtx.y, vec.y, res.y);
    Scale(mtx.z, vec.z, res.z);
}

// is the sphere in front of or on the plane?
inline bool operator>=(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) >= s.GetRadius();
}

// is the sphere behind the plane?
inline bool operator<(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) < -s.GetRadius();
}
