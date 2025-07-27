#pragma once
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/TextStream.h"

class Vector2 {
public:
    Vector2() {}
    Vector2(float xx, float yy) : x(xx), y(yy) {}
    Vector2(const Vector2 &vec) : x(vec.x), y(vec.y) {}

    void Set(float xx, float yy) {
        x = xx;
        y = yy;
    }

    // Vector2& operator*=(float f) {
    //     x *= f; y *= f;
    //     return *this;
    // }
    // there's no way this returns a float what
    // according to RndCam::WorldToScreen it does
    float operator*=(float f) {
        x *= f;
        y *= f;
        return f;
    }

    void Zero() { x = y = 0.0f; }

    Vector2 &operator/=(float f) {
        x /= f;
        y /= f;
        return *this;
    }

    Vector2 &operator+=(const Vector2 &v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    bool operator==(const Vector2 &v) const { return x == v.x && y == v.y; }

    bool operator!() const { return x == 0.0f && y == 0.0f; }

    float x;
    float y;
};

inline BinStream &operator<<(BinStream &bs, const Vector2 &vec) {
    bs << vec.x << vec.y;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Vector2 &vec) {
    bs >> vec.x >> vec.y;
    return bs;
}

class Vector3 {
protected:
    static Vector3 sX;
    static Vector3 sY;
    static Vector3 sZ;
    static Vector3 sZero;

public:
    float x;
    float y;
    float z;

    Vector3() {}
    Vector3(float f1, float f2, float f3) : x(f1), y(f2), z(f3) {}
    // Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}

    // used during decompression of CharBones vectors
    Vector3(short *s) {
        x = s[0] * 0.000030518509f * 1300.0f;
        y = s[1] * 0.000030518509f * 1300.0f;
        z = s[2] * 0.000030518509f * 1300.0f;
    }

    void Set(float f1, float f2, float f3) {
        x = f1;
        y = f2;
        z = f3;
    }
    void Zero() { x = y = z = 0.0f; }

    float X() const { return x; }
    float Y() const { return y; }
    float Z() const { return z; }

    Vector3 &operator+=(const Vector3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector3 &operator-=(const Vector3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vector3 &operator*=(float f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    Vector3 &operator*=(const Vector3 &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    Vector3 &operator/=(float f) {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    const float &operator[](int i) const { return *(&x + i); }
    float &operator[](int i) { return *(&x + i); }
    bool operator==(const Vector3 &v) const { return x == v.x && y == v.y && z == v.z; }
    bool IsZero() const { return x == 0 && y == 0 && z == 0; }
    bool operator!=(const Vector3 &v) const { return x != v.x || y != v.y || z != v.z; }

private:
    u32 PAD; // should NEVER be used!!!! for simd alignment!!!
};

inline BinStream &operator<<(BinStream &bs, const Vector3 &vec) {
    bs << vec.x << vec.y << vec.z;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Vector3 &vec) {
    bs >> vec.x >> vec.y >> vec.z;
    return bs;
}

TextStream &operator<<(TextStream &, const Vector3 &);

class Vector4 {
protected:
    static Vector4 sX;
    static Vector4 sY;
    static Vector4 sZ;
    static Vector4 sW;
    static Vector4 sZero;

public:
    float x;
    float y;
    float z;
    float w;

    Vector4() {}
    Vector4(float f1, float f2, float f3, float f4) : x(f1), y(f2), z(f3), w(f4) {}
    void Set(float f1, float f2, float f3, float f4) {
        x = f1;
        y = f2;
        z = f3;
        w = f4;
    }

    // Vector4(const Vector4 &);
};

inline BinStream &operator>>(BinStream &bs, Vector4 &vec) {
    bs >> vec.x >> vec.y >> vec.z >> vec.w;
    return bs;
}
