#pragma once
#include "math/Vec.h"

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

#include "Plane.inl"
