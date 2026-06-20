#pragma once
#include "Vec.h"
#include "utl/BinStream.h"
#include "math/Utl.h"

#pragma region Vector2

inline BinStream &operator<<(BinStream &bs, const Vector2 &vec) {
    bs << vec.x << vec.y;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Vector2 &vec) {
    bs >> vec.x >> vec.y;
    return bs;
}

inline bool NearlyEqual(const Vector2 &v1, const Vector2 &v2, float max_diff) {
    return std::fabs(v1.x - v2.x) < max_diff && std::fabs(v1.y - v2.y) < max_diff;
}

inline float Length(const Vector2 &v) { return std::sqrt(v.x * v.x + v.y * v.y); }

inline float Average(const Vector2 &v) { return (v.x + v.y) / 2; }

inline float Cross(const Vector2 &v1, const Vector2 &v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

inline void Interp(const Vector2 &v1, const Vector2 &v2, float f, Vector2 &res) {
    res.Set(Interp(v1.x, v2.x, f), Interp(v1.y, v2.y, f));
}

#pragma endregion
#pragma region Vector3

inline BinStream &operator<<(BinStream &bs, const Vector3 &vec) {
    bs << vec.x << vec.y << vec.z;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Vector3 &vec) {
    bs >> vec.x >> vec.y >> vec.z;
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

inline float Dot(const Vector3 &v1, const Vector3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline void Cross(const Vector3 &v1, const Vector3 &v2, Vector3 &dst) {
    dst.Set(
        v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x
    );
}

inline void Normalize(const Vector3 &in, Vector3 &out) {
    float len = Length(in);
    float inv = len != 0 ? 1 / len : 0;
    Scale(in, inv, out);
}

inline void NormalizeScale(const Vector3 &in, float scalar, Vector3 &out) {
    float len = Length(in);
    float inv = len != 0 ? 1 / len : 0;
    Scale(in, inv * scalar, out);
}

inline void Negate(const Vector3 &v, Vector3 &vres) { vres.Set(-v.x, -v.y, -v.z); }

inline float Distance(const Vector3 &v1, const Vector3 &v2) {
    Vector3 diff;
    Subtract(v1, v2, diff);
    return Length(diff);
}

inline float DistanceSquared(const Vector3 &v1, const Vector3 &v2) {
    Vector3 diff;
    Subtract(v1, v2, diff);
    return LengthSquared(diff);
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

#pragma endregion
#pragma region Vector4

inline BinStream &operator<<(BinStream &bs, const Vector4 &vec) {
    bs << vec.x << vec.y << vec.z << vec.w;
    return bs;
}
inline BinStream &operator>>(BinStream &bs, Vector4 &vec) {
    bs >> vec.x >> vec.y >> vec.z >> vec.w;
    return bs;
}

#pragma endregion
