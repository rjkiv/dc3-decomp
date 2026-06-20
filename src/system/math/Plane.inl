#pragma once
#include "Plane.h"
#include "Sphere.h"
#include "Mtx.h"

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

void Multiply(const Plane &, const Transform &, Plane &);

// is the sphere in front of or on the plane?
inline bool operator>=(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) >= s.radius;
}

// is the sphere behind the plane?
inline bool operator<(const Sphere &s, const Plane &p) {
    return p.Dot(s.center) < -s.radius;
}
