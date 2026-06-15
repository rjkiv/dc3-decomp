#pragma once
#include "math/Sphere.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "math/Trig.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

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
        // maybe this one isn't macro-ified
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
        Matrix4(const Transform &xfm);

        Matrix4(
            const Vector4 &v1, const Vector4 &v2, const Vector4 &v3, const Vector4 &v4
        ) {
            m[0] = v1;
            m[1] = v2;
            m[2] = v3;
            m[3] = v4;
        }

        Matrix4 &Zero();

        COPY_OPERATOR(Matrix4)
        Vector3 Col3(int idx) const { return Vector3(m[0][idx], m[1][idx], m[2][idx]); }

        static const Hmx::Matrix4 &ID() { return sID; }

        // RBVR says this is an array
        Vector4 m[4];
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

class Transform {
private:
    static Transform sID;
    //   private: static Transform sZero;

public:
    class Hmx::Matrix3 m;
    class Vector3 v;

    // all of these are weak
    Transform() {}

    Transform(const Hmx::Matrix3 &mtx, const Vector3 &vec) : m(mtx), v(vec) {}

    // Transform(const Transform &tf);
    COPY_OPERATOR(Transform)

    void Reset() {
        m.Identity();
        v.Zero();
    }

    void Set(const Hmx::Matrix3 &mtx, const Vector3 &vec) {
        m = mtx;
        v = vec;
    }

    void LookAt(const Vector3 &v1, const Vector3 &v2) {
        Subtract(v1, v, m.y);
        m.z = v2;
        Normalize(m, m);
    }

    void Zero() {
        m.Zero();
        v.Zero();
    }

    bool operator==(const Transform &tf) const { return m == tf.m && v == tf.v; }
    bool operator!=(const Transform &tf) const { return m != tf.m || v != tf.v; }

    //   public: Vector3& operator[](int32_t);
    //   public: const Vector3& operator[](int32_t) const;
    //   public: const float*AsArray()[3] const;
    //   public: float*AsArray()[3];

    //   public: Transform operator*(const Hmx::Matrix3&) const;
    //   public: Transform operator*(const Transform&) const;
    //   public: Transform& operator*=(const Hmx::Matrix3&);
    //   public: Transform& operator*=(const Transform&);
    //   public: String Token(bool) const;
    //   public: bool SetFromToken(const char*);
    //   public: Transform& operator=(Transform&&);

    //   public: static const Transform& GetZero();
    static const Transform &GetIdentity() { return sID; }
};

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
    QuatXfm() {}
    QuatXfm(const Transform &);

    Vector3 v;
    Hmx::Quat q;
};

/// An infinite plane, defined as its normal and distance from origin.
// Defined as ax+by+cz=-d.
class Plane {
public:
    Plane() {}
    //   public: Plane(float, float, float, float);
    Plane(const Vector3 &v1, const Vector3 &v2) { Set(v1, v2); }
    //   public: Plane(const Vector3&, const Vector3&, const Vector3&);

    void Set(const Vector3 &v1, const Vector3 &v2) {
        a = v2.x;
        b = v2.y;
        c = v2.z;
        d = -::Dot(v2, v1);
    }

    void Set(const Vector3 &, const Vector3 &, const Vector3 &);

    void Set(float nx, float ny, float nz, float dist) {
        a = nx;
        b = ny;
        c = nz;
        d = dist;
    }

    /// Returns the dot product between `vec` and the plane normal.
    float Dot(const Vector3 &vec) const { return a * vec.x + b * vec.y + c * vec.z + d; }

    /// Returns the point on the plane closest to the origin.
    // (a,b,c) must be normalized.
    Vector3 On() const {
        Vector3 ret;
        float scalar = -d / (a * a + b * b + c * c);
        ret.Set(a * scalar, b * scalar, c * scalar);
        return ret;
    }

    //   public: const Vector3& Normal() const;
    //   public: Vector3& Normal();
    //   public: bool operator==(const Plane&) const;
    //   public: bool operator!=(const Plane&) const;

    Vector4 &AsVector4() { return reinterpret_cast<Vector4 &>(*this); }
    const Vector4 &AsVector4() const { return reinterpret_cast<const Vector4 &>(*this); }

    //   public: void Project(const Vector3&, Vector3&) const;

    float a, b, c, d;
};

inline bool operator<=(const Vector3 &v, const Plane &p) { return 0 <= p.Dot(v); }

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

void Interp(const Hmx::Matrix3 &, const Hmx::Matrix3 &, float, Hmx::Matrix3 &);

inline void
FasterInterp(const Hmx::Quat &q1, const Hmx::Quat &q2, float f, Hmx::Quat &qres) {
    qres.x = Interp(q1.x, q2.x, f);
    qres.y = Interp(q1.y, q2.y, f);
    qres.z = Interp(q1.z, q2.z, f);
    qres.w = Interp(q1.w, q2.w, f);
}

void Multiply(const Hmx::Matrix3 &, const Hmx::Matrix3 &, Hmx::Matrix3 &);
void Multiply(const Transform &, const Hmx::Matrix3 &, Transform &);
void Multiply(const Vector4 &, const Hmx::Matrix4 &, Vector4 &);

inline void MultiplyTranspose(const Vector3 &v, const Transform &t, Vector3 &out) {
    Subtract(v, t.v, out);
    out.Set(Dot(out, t.m.x), Dot(out, t.m.y), Dot(out, t.m.z));
}

void Multiply(const Plane &, const Transform &, Plane &);

inline void Multiply(const Hmx::Quat &q1, const Hmx::Quat &q2, Hmx::Quat &qres) {
    qres.Set(
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
        q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
    );
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

void Transpose(const Hmx::Matrix4 &, Hmx::Matrix4 &);

inline void Multiply(const Frustum &fin, const Transform &tf, Frustum &fout) {
    Multiply(fin.front, tf, fout.front);
    Multiply(fin.back, tf, fout.back);
    Multiply(fin.left, tf, fout.left);
    Multiply(fin.right, tf, fout.right);
    Multiply(fin.top, tf, fout.top);
    Multiply(fin.bottom, tf, fout.bottom);
}

inline void Transpose(const Hmx::Matrix3 &in, Hmx::Matrix3 &out) {
    out.Set(in.x.x, in.y.x, in.z.x, in.x.y, in.y.y, in.z.y, in.x.z, in.y.z, in.z.z);
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

// is the sphere in front of or on the plane?
inline bool operator>=(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) >= s.radius;
}

// is the sphere behind the plane?
inline bool operator<(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) < -s.radius;
}

inline void ScaleAddEq(Hmx::Matrix3 &dst, const Hmx::Matrix3 &src, float scalar) {
    ScaleAddEq(dst.x, src.x, scalar);
    ScaleAddEq(dst.y, src.y, scalar);
    ScaleAddEq(dst.z, src.z, scalar);
}

inline void ScaleAddEq(Transform &dst, const Transform &src, float scalar) {
    ScaleAddEq(dst.m, src.m, scalar);
    ScaleAddEq(dst.v, src.v, scalar);
}

void ScaleAddEq(Hmx::Quat &, const Hmx::Quat &, float);
