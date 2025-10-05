#pragma once
#include "math/Vec.h"
#include "math/Trig.h"
#include "utl/BinStream.h"
#include "utl/TextStream.h"

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

        static const Hmx::Matrix3 &ID() { return sID; }
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

        bool operator!=(const Quat &q) const {
            return x != q.x || y != q.y || z != q.z || w != q.w;
        }

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

TextStream &operator<<(TextStream &, const Transform &);

inline BinStream &operator<<(BinStream &bs, const Transform &tf) {
    bs << tf.m << tf.v;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Transform &tf) {
    bs >> tf.m >> tf.v;
    return bs;
}

class QuatXfm {
public:
    Vector3 v;
    Hmx::Quat q;
};

class Plane {
public:
    Plane() {}
    Plane(const Vector3 &v1, const Vector3 &v2) { Set(v1, v2); }

    void Set(const Vector3 &, const Vector3 &);
    float Dot(const Vector3 &vec) const { return a * vec.x + b * vec.y + c * vec.z + d; }
    Vector3 On() const;

    float a, b, c, d;
};

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

void Normalize(const Hmx::Matrix3 &, Hmx::Matrix3 &);
void Multiply(const Hmx::Matrix3 &, const Hmx::Matrix3 &, Hmx::Matrix3 &);
void MultiplyInverse(const Transform &, const Transform &, Transform &);
void Multiply(const Transform &, const Transform &, Transform &);
void MultiplyTranspose(const Vector3 &, const Transform &, Vector3 &);
void Multiply(const Vector3 &, const Transform &, Vector3 &);

void Invert(const Transform &, Transform &);
void Invert(const Hmx::Matrix4 &, Hmx::Matrix4 &);
void Transpose(const Hmx::Matrix4 &, Hmx::Matrix4 &);
void Multiply(const Frustum &, const Transform &, Frustum &);

void MakeRotMatrix(const Hmx::Quat &, Hmx::Matrix3 &);

void Scale(const Hmx::Matrix3 &mtx, const Vector3 &vec, Hmx::Matrix3 &res);

// void Scale(const Hmx::Matrix3 &mtx, const Vector3 &vec, Hmx::Matrix3 &res) {
//     Scale(mtx.x, vec.x, res.x);
//     Scale(mtx.y, vec.y, res.y);
//     Scale(mtx.z, vec.z, res.z);
// }

// // void Scale(Matrix3 *param_1,Vector3 *param_2,Matrix3 *param_3)

// // {

// //   (param_3->x).x = (param_1->x).x * param_2->x;
// //   (param_3->x).z = (param_1->x).z * param_2->z;
// //   (param_3->x).y = (param_1->x).y * param_2->y;

// //   (param_3->y).x = (param_1->y).x * param_2->x;
// //   (param_3->y).y = (param_1->y).y * param_2->y;
// //   (param_3->y).z = (param_1->y).z * param_2->z;

// //   (param_3->z).x = (param_1->z).x * param_2->x;
// //   (param_3->z).y = (param_1->z).y * param_2->y;
// //   (param_3->z).z = (param_1->z).z * param_2->z;
// //   return;
// // }

inline void Scale(const Vector3 &vec, const Hmx::Matrix3 &mtx, Hmx::Matrix3 &res) {
    Scale(mtx.x, vec.x, res.x);
    Scale(mtx.y, vec.y, res.y);
    Scale(mtx.z, vec.z, res.z);
}
