#pragma once
#include "math/Mtx.h"
#include "math/Sphere.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"

class Segment {
public:
    Segment() {}

    Segment &operator=(const Segment &s) {
        memcpy(this, &s, sizeof(*this));
        return *this;
    }

    Vector3 start;
    Vector3 end;
};

namespace Hmx {
    class Rect {
    public:
        Rect() {}
        Rect(float xx, float yy, float ww, float hh) : x(xx), y(yy), w(ww), h(hh) {}
        void Set(float xx, float yy, float ww, float hh) {
            x = xx;
            y = yy;
            w = ww;
            h = hh;
        }
        float x, y, w, h;
    };

    class Polygon {
    public:
        Polygon() {}
        ~Polygon() {}
        std::vector<Vector2> points;
    };

    class Ray {
    public:
        Vector2 base, dir;
    };
}

inline BinStream &operator<<(BinStream &bs, const Hmx::Rect &rect) {
    bs << rect.x << rect.y << rect.w << rect.h;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::Rect &rect) {
    bs >> rect.x >> rect.y >> rect.w >> rect.h;
    return bs;
}

class Triangle {
public:
    Vector3 origin;
    Hmx::Matrix3 frame;
};

class Box {
public:
    Box() {}
    Box(const Vector3 &min, const Vector3 &max) : mMin(min), mMax(max) {}

    void Set(const Vector3 &min, const Vector3 &max) {
        mMin = min;
        mMax = max;
    }

    void GrowToContain(const Vector3 &vec, bool b);
    void Extend(float);
    bool Contains(const Vector3 &) const;
    bool Contains(const Sphere &) const;
    bool Contains(const Triangle &) const;
    float SurfaceArea() const;
    float Volume() const;
    bool Clamp(Vector3 &);

    Vector3 mMin;
    Vector3 mMax;
};

inline BinStream &operator<<(BinStream &bs, const Box &box) {
    bs << box.mMin << box.mMax;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Box &box) {
    bs >> box.mMin >> box.mMax;
    return bs;
}

class BSPNode {
public:
    BSPNode() : left(nullptr), right(nullptr) {}
    ~BSPNode() {
        delete left;
        delete right;
    }

    POOL_OVERLOAD(BSPNode, 0x216);

    Plane plane; // 0x0
    BSPNode *left; // 0x10 yes they're called front/back but BSP works L/R, not F/B
    BSPNode *right; // 0x14
};

class BSPFace {
public:
    void Set(const Vector3 &, const Vector3 &, const Vector3 &);

    Hmx::Polygon p; // 0x0
    Transform t; // 0xc
    float area; // 0x4c
    std::list<Plane> planes; // 0x50
};

void NumNodes(const BSPNode *, int &, int &);
BinStream &operator<<(BinStream &, const BSPNode *);
BinStream &operator>>(BinStream &, BSPNode *&);
bool MakeBSPTree(BSPNode *&, std::list<BSPFace> &, int);
bool CheckBSPTree(const BSPNode *, const Box &);
void Multiply(const Box &, float, Box &);
bool Intersect(const Transform &, const Hmx::Polygon &, const BSPNode *);
void MultiplyEq(BSPNode *, const Transform &);
void Multiply(const Plane &, const Transform &, Plane &);

inline void Multiply(const Sphere &s, const Transform &t, Sphere &out) {
    Multiply(s.center, t, out.center);
    float len = Max(LengthSquared(t.m.z), LengthSquared(t.m.y), LengthSquared(t.m.x));
    len = std::sqrt(len);
    if (NearlyOne(len)) {
        len = 1;
    }
    out.radius = s.radius * len;
}

void Intersect(const Hmx::Ray &, const Hmx::Ray &, Vector2 &);
void Intersect(const Transform &, const Plane &, Hmx::Ray &);
bool Intersect(const Transform &, const Hmx::Polygon &, const BSPNode *);
bool Intersect(const Segment &, const Triangle &, bool, float &);
bool Intersect(const Segment &, const BSPNode *, float &, Plane &);
bool Intersect(const Segment &, const Sphere &);
bool Intersect(const Vector3 &, const BSPNode *);
bool Intersect(const Vector3 &, const Vector3 &, const Triangle &, float &);
bool Intersect(const Vector3 &, const Vector3 &, const Box &, float &, float &);
bool Intersect(const Plane &, const Box &);
bool Intersect(const Triangle &, const Box &);


DataNode SetBSPParams(DataArray *da);
void GeoInit();

inline void CalcBoxCenter(Vector3 &center, const Box &box) {
    Add(box.mMin, box.mMax, center);
    Scale(center, 0.5f, center);
}

extern float gUnitsPerMeter;
