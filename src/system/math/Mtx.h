#pragma once
#include "math/Sphere.h"
#include "math/Vec.h"
#include "utl/BinStream.h"

class Transform;

namespace Hmx {
    // size 0x10
    class Matrix2 {
    private:
        static Matrix2 sID;

    public:
        Matrix2(const Vector2 &xIn, const Vector2 &yIn) : x(xIn), y(yIn) {}

        Vector2 x; // 0x0
        Vector2 y; // 0x8
    };

    class Matrix3 {
    public:
        // all of these are weak
        Matrix3() {}
        Matrix3(const Vector3 &xIn, const Vector3 &yIn, const Vector3 &zIn)
            : x(xIn), y(yIn), z(zIn) {}

        // clang-format off
        Matrix3(
            float xx, float xy, float xz,
            float yx, float yy, float yz,
            float zx, float zy, float zz
        )
            : x(xx, xy, xz), y(yx, yy, yz), z(zx, zy, zz) {}

        void Set(
            float xx, float xy, float xz,
            float yx, float yy, float yz,
            float zx, float zy, float zz
        ) {
            x.Set(xx, xy, xz);
            y.Set(yx, yy, yz);
            z.Set(zx, zy, zz);
        }
        // clang-format on
        void Set(const Vector3 &xIn, const Vector3 &yIn, const Vector3 &zIn) {
            x = xIn;
            y = yIn;
            z = zIn;
        }
        void Zero() {
            x.Zero();
            y.Zero();
            z.Zero();
        }
        void Identity() {
            x.Set(1.0f, 0.0f, 0.0f);
            y.Set(0.0f, 1.0f, 0.0f);
            z.Set(0.0f, 0.0f, 1.0f);
        }

        Vector3 &operator[](int i) { return *(&x + i); }

        bool operator==(const Matrix3 &mtx) const {
            return x == mtx.x && y == mtx.y && z == mtx.z;
        }

        bool operator!=(const Matrix3 &mtx) const {
            return x != mtx.x || y != mtx.y || z != mtx.z;
        }

        static const Hmx::Matrix3 &GetIdentity() { return sID; }

        Vector3 x; // 0x0
        Vector3 y; // 0x10
        Vector3 z; // 0x20

    private:
        static Matrix3 sID;
    };

    class Matrix4 {
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

        Vector3 Col3(int idx) const { return Vector3(m[0][idx], m[1][idx], m[2][idx]); }

        static const Hmx::Matrix4 &ID() { return sID; }

        // RBVR says this is an array
        Vector4 m[4];

    private:
        static Matrix4 sID;
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

    void Reset() {
        m.Identity();
        v.Zero();
    }

    void Set(const Hmx::Matrix3 &mtx, const Vector3 &vec) {
        m = mtx;
        v = vec;
    }

    void LookAt(const Vector3 &v1, const Vector3 &v2);

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

void Multiply(const Plane &, const Transform &, Plane &);

inline void Multiply(const Frustum &fin, const Transform &tf, Frustum &fout) {
    Multiply(fin.front, tf, fout.front);
    Multiply(fin.back, tf, fout.back);
    Multiply(fin.left, tf, fout.left);
    Multiply(fin.right, tf, fout.right);
    Multiply(fin.top, tf, fout.top);
    Multiply(fin.bottom, tf, fout.bottom);
}

// is the sphere in front of or on the plane?
inline bool operator>=(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) >= s.radius;
}

// is the sphere behind the plane?
inline bool operator<(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) < -s.radius;
}

#include "Mtx.inl"
