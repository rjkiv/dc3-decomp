#pragma once
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/TextStream.h"
#include "math/Utl.h"
#include <cmath>

class Vector2 {
public:
    Vector2() {}
    Vector2(float xx, float yy) : x(xx), y(yy) {}

    void Set(float xx, float yy) {
        x = xx;
        y = yy;
    }

    Vector2 &operator*=(float f) {
        x *= f;
        y *= f;
        return *this;
    }

    void Zero() { x = y = 0; }

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

    void Set(float f1, float f2, float f3) {
        x = f1;
        y = f2;
        z = f3;
    }
    void Zero() { x = y = z = 0; }

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

    const float &operator[](int i) const {
        MILO_ASSERT_RANGE(i, 0, 3, 0x122);
        return *(&x + i);
    }

    float &operator[](int i) {
        MILO_ASSERT_RANGE(i, 0, 3, 0x127);
        return *(&x + i);
    }

    //   public: static const Vector3& GetXAxis();
    //   public: static const Vector3& GetYAxis();
    //   public: static const Vector3& GetZAxis();
    static const Vector3 &GetZero() { return sZero; }

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

    static const Vector4 &GetZero() { return sZero; }

    const float &operator[](int i) const {
        MILO_ASSERT_RANGE(i, 0, 4, 0x1AC);
        return *(&x + i);
    }
};

inline BinStream &operator<<(BinStream &bs, const Vector4 &vec) {
    bs << vec.x << vec.y << vec.z << vec.w;
    return bs;
}
inline BinStream &operator>>(BinStream &bs, Vector4 &vec) {
    bs >> vec.x >> vec.y >> vec.z >> vec.w;
    return bs;
}

inline bool NearlyEqual(const Vector3 &v1, const Vector3 &v2, float max_diff) {
    return std::fabs(v1.x - v2.x) < max_diff && std::fabs(v1.y - v2.y) < max_diff
        && std::fabs(v1.z - v2.z) < max_diff;
}

inline void Scale(const Vector3 &v1, float f, Vector3 &dst) {
    dst.Set(v1.x * f, v1.y * f, v1.z * f);
}

inline void Scale(const Vector3 &v1, const Vector3 &v2, Vector3 &dst) {
    dst.Set(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

inline void Add(const Vector3 &v1, const Vector3 &v2, Vector3 &dst) {
    dst.Set(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

inline void Subtract(const Vector3 &v1, const Vector3 &v2, Vector3 &dst) {
    dst.Set(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

inline float LengthSquared(const Vector3 &v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

inline float Length(const Vector3 &v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float Average(const Vector2 &v) { return (v.x + v.y) / 2; }

inline float Dot(const Vector3 &v1, const Vector3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline float Cross(const Vector2 &v1, const Vector2 &v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

inline void Cross(const Vector3 &v1, const Vector3 &v2, Vector3 &dst) {
    dst.Set(
        v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x
    );
}

inline void Normalize(const Vector3 &in, Vector3 &out) {
    float inv = 0;
    float len = Length(in);
    if (len != 0) {
        inv = 1.0f / len;
    }
    Scale(in, inv, out);
}

void NormalizeScale(const Vector3 &, float, Vector3 &);

inline void Negate(const Vector3 &v, Vector3 &vres) { vres.Set(-v.x, -v.y, -v.z); }

inline void Interp(const Vector2 &v1, const Vector2 &v2, float f, Vector2 &res) {
    res.Set(Interp(v1.x, v2.x, f), Interp(v1.y, v2.y, f));
}

inline void Interp(const Vector3 &v1, const Vector3 &v2, float f, Vector3 &dst) {
    if (f == 0) {
        dst = v1;
        return;
    } else if (f == 1) {
        dst = v2;
        return;
    } else {
        dst.Set(Interp(v1.x, v2.x, f), Interp(v1.y, v2.y, f), Interp(v1.z, v2.z, f));
    }
}

inline float Distance(const Vector3 &v1, const Vector3 &v2) {
    Vector3 diff;
    Subtract(v1, v2, diff);
    return Length(diff);
}

inline void ScaleAdd(const Vector3 &v1, const Vector3 &v2, float f, Vector3 &vres) {
    vres.x = v2.x * f + v1.x;
    vres.y = v2.y * f + v1.y;
    vres.z = v2.z * f + v1.z;
}

inline void ScaleAddEq(Vector3 &vres, const Vector3 &v2, float f) {
    vres.x += v2.x * f;
    vres.y += v2.y * f;
    vres.z += v2.z * f;
}

// actually defined elsewhere and not in here! (Geo.cpp)
void ClosestPoint(const Vector3 &, const Vector3 &, const Vector3 &, Vector3 *);
