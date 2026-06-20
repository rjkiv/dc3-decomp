#pragma once
#include "os/Debug.h"
#include "utl/TextStream.h"
#include "math/Utl.h"
#include <cmath>

class Vector2 {
public:
    Vector2() {}
    Vector2(float xIn, float yIn) : x(xIn), y(yIn) {}

    const float &operator[](int i) const { return *(&x + i); }
    float &operator[](int i) { return *(&x + i); }

    // clang-format off
    void Set(float xIn, float yIn) { x = xIn; y = yIn; }
    void Zero() { x = y = 0; }

    Vector2 &operator+=(const Vector2 &v) { x += v.x; y += v.y; return *this; }
    Vector2 &operator-=(const Vector2 &v) { x -= v.x; y -= v.y; return *this; }
    Vector2 &operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2 &operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }
    // clang-format on

    bool operator==(const Vector2 &v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vector2 &v) const { return x != v.x || y != v.y; }

    // maybe these are used, maybe not, i dunno
    // i'm leaning towards not tho

    // float const *AsArray() const;
    // float *AsArray();
    // void Set(float const *);
    float Length() const { return sqrtf(x * x + y * y); }
    // float LengthSquared() const;
    // float Distance(Vector2 const &) const;
    // float DistanceSquared(Vector2 const &) const;
    // float Dot(Vector2 const &) const;
    // float Cross(Vector2 const &) const;
    // Vector2 Expand() const;
    // Vector2 Narrow() const;

    float x; // 0x0
    float y; // 0x4
};

TextStream &operator<<(TextStream &ts, const Vector2 &v);

class Vector3 {
public:
    Vector3() {}
    Vector3(float xIn, float yIn, float zIn) : x(xIn), y(yIn), z(zIn) {}

    const float &operator[](int i) const {
        MILO_ASSERT_RANGE(i, 0, 3, 0x122);
        return *(&x + i);
    }

    float &operator[](int i) {
        MILO_ASSERT_RANGE(i, 0, 3, 0x127);
        return *(&x + i);
    }

    // clang-format off
    void Set(float xIn, float yIn, float zIn) { x = xIn; y = yIn; z = zIn; }
    void Zero() { x = y = z = 0; }

    Vector3 &operator+=(const Vector3 &v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3 &operator-=(const Vector3 &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector3 &operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vector3 &operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }
    // clang-format on

    bool operator==(const Vector3 &v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const Vector3 &v) const { return x != v.x || y != v.y || z != v.z; }

    // maybe these are used, maybe not, i dunno
    // i'm leaning towards not tho

    // float const *AsArray() const;
    // float *AsArray();
    // Vector2 &AsVector2();
    // Vector2 const &AsVector2() const;
    // float Length() const;
    // float LengthSquared() const;
    // float Distance(Vector3 const &) const;
    // float DistanceSquared(Vector3 const &) const;
    // Vector3 Cross(Vector3 const &) const;
    // float Dot(Vector3 const &) const;
    // Vector3 Expand() const;
    // Vector3 Narrow() const;

    static const Vector3 &GetXAxis() { return sX; }
    static const Vector3 &GetYAxis() { return sY; }
    static const Vector3 &GetZAxis() { return sZ; }
    static const Vector3 &GetZero() { return sZero; }

    float x; // 0x0
    float y; // 0x4
    float z; // 0x8
private:
    u32 PAD; // should NEVER be used!!!! for simd alignment!!!
protected:
    static Vector3 sX;
    static Vector3 sY;
    static Vector3 sZ;
    static Vector3 sZero;
};

TextStream &operator<<(TextStream &, const Vector3 &);

class ShortVector3 {
public:
    void Set(const Vector3 &);

    void ToVector3(Vector3 &v) const {
        v.Set(x * 0.039674062f, y * 0.039674062f, z * 0.039674062f);
    }

    static short ToShort(float f) {
        float value = f * 0.00076923077f;
        value *= 32767.0f;
        value += 0.5f;
        return floor(Clamp(-32767.0f, 32767.0f, value));
    }

    short x;
    short y;
    short z;
};

class Vector4 {
public:
    Vector4() {}
    Vector4(float xIn, float yIn, float zIn, float wIn)
        : x(xIn), y(yIn), z(zIn), w(wIn) {}
    void Set(float xIn, float yIn, float zIn, float wIn) {
        x = xIn;
        y = yIn;
        z = zIn;
        w = wIn;
    }

    static const Vector4 &GetZero() { return sZero; }

    const float &operator[](int i) const {
        MILO_ASSERT_RANGE(i, 0, 4, 0x1AC);
        return *(&x + i);
    }

    float x; // 0x0
    float y; // 0x4
    float z; // 0x8
    float w; // 0xc

protected:
    static Vector4 sX;
    static Vector4 sY;
    static Vector4 sZ;
    static Vector4 sW;
    static Vector4 sZero;
};

// actually defined elsewhere and not in here! (Geo.cpp)
void ClosestPoint(const Vector3 &, const Vector3 &, const Vector3 &, Vector3 *);

#include "Vec.inl"
