#include "math/Geo.h"
#include "Vec.h"
#include "math/Mtx.h"
#include "math/Sphere.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/DataFunc.h"
#include "os/System.h"
#include "utl/BinStream.h"

float gUnitsPerMeter = 39.370079f;
float gBSPPosTol = 0.01f;
float gBSPDirTol = 0.985f;
int gBSPMaxDepth = 20;
int gBSPMaxCandidates = 40;
float gBSPCheckScale = 1.1f;

void NumNodes(const BSPNode *node, int &num, int &maxDepth) {
    static int depth = 0;
    if (node) {
        depth++;
        if (depth == 1) {
            num = 0;
            maxDepth = 1;
        } else if (depth > maxDepth) {
            maxDepth = depth;
        }
        NumNodes(node->left, num, maxDepth);
        NumNodes(node->right, num, maxDepth);
        num++;
        depth--;
    }
}

BinStream &operator<<(BinStream &bs, const BSPNode *node) {
    if (node) {
        bs << true;
        bs << node->plane << node->left << node->right;
    } else {
        bs << false;
    }
    return bs;
}

BinStream &operator>>(BinStream &bs, BSPNode *&node) {
    unsigned char nodeExists;
    bs >> nodeExists;
    if (nodeExists) {
        node = new BSPNode();
        bs >> node->plane >> node->left >> node->right;
    } else {
        node = nullptr;
    }
    return bs;
}

void Box::Extend(float scale) {
    mMin.x -= scale;
    mMin.y -= scale;
    mMin.z -= scale;
    mMax.x += scale;
    mMax.y += scale;
    mMax.z += scale;
}

bool Box::Contains(const Vector3 &v) const {
    return mMin.x <= v.x && mMin.y <= v.y && mMin.z <= v.z && mMax.x >= v.x
        && mMax.y >= v.y && mMax.z >= v.z;
}

bool Box::Contains(const Sphere &s) const {
    return mMin.x <= s.center.x - s.radius && mMin.y <= s.center.y - s.radius
        && mMin.z <= s.center.z - s.radius && mMax.x >= s.center.x + s.radius
        && mMax.y >= s.center.y + s.radius && mMax.z >= s.center.z + s.radius;
}

bool Box::Contains(const Triangle &t) const {
    Vector3 v1 = t.origin;
    Vector3 v2(
        t.frame.x.x + t.origin.x, t.frame.x.y + t.origin.y, t.frame.x.z + t.origin.z
    );
    Vector3 v3(
        t.frame.y.x + t.origin.x, t.frame.y.y + t.origin.y, t.frame.y.z + t.origin.z
    );
    return Contains(v1) && Contains(v2) && Contains(v3);
}

float Box::SurfaceArea() const {
    float x = mMax.x - mMin.x;
    float y = mMax.y - mMin.y;
    float z = mMax.z - mMin.z;
    float xy = x * y * 2;
    float xz = x * z * 2;
    float yz = y * z * 2;
    return xy + xz + yz;
}

float Box::Volume() const {
    return (mMax.z - mMin.z) * (mMax.y - mMin.y) * (mMax.x - mMin.x);
}

void Box::GrowToContain(const Vector3 &vec, bool b) {
    if (b) {
        mMin = mMax = vec;
    } else
        for (int i = 0; i < 3; i++) {
            MinEq(mMin[i], vec[i]);
            MaxEq(mMax[i], vec[i]);
        }
}

bool Box::Clamp(Vector3 &v) {
    return ClampEq(v.x, mMin.x, mMax.x) | ClampEq(v.y, mMin.y, mMax.y)
        | ClampEq(v.z, mMin.z, mMax.z);
}

void Normalize(const Plane &in, Plane &out) {
    float mult = 0;
    float len = std::sqrt(in.a * in.a + in.b * in.b + in.c * in.c);
    if (len != 0) {
        mult = 1 / len;
    }
    out.Set(in.a * mult, in.b * mult, in.c * mult, in.d * mult);
}

void ClosestPoint(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, Vector3 *vout) {
    Vector3 diff31, diff21;
    Subtract(v2, v1, diff21);
    Subtract(v3, v1, diff31);
    float f5 = Dot(diff31, diff21);
    if (f5 <= 0) {
        *vout = v1;
    } else {
        float dot21 = Dot(diff21, diff21);
        if (f5 > dot21) {
            *vout = v2;
        } else {
            Scale(diff21, f5 / dot21, diff21);
            Add(v1, diff21, *vout);
        }
    }
}

void Plane::Set(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
    Vector3 diff21, diff31, cross;
    Subtract(v2, v1, diff21);
    Subtract(v3, v1, diff31);
    Cross(diff21, diff31, cross);
    Normalize(cross, cross);
    a = cross.x;
    b = cross.y;
    c = cross.z;
    d = -::Dot(cross, v1);
}

void SetBSPParams(float f1, float f2, int r3, int r4, float f3) {
    gBSPPosTol = f1;
    gBSPDirTol = f2;
    gBSPMaxDepth = r3;
    gBSPMaxCandidates = r4;
    gBSPCheckScale = f3;
}

DataNode SetBSPParams(DataArray *da) {
    SetBSPParams(da->Float(1), da->Float(2), da->Int(3), da->Int(4), da->Float(5));
    return 0;
}

void GeoInit() {
    DataArray *cfg = SystemConfig("math");
    float scale = cfg->FindArray("bsp_check_scale")->Float(1);
    int candidates = cfg->FindArray("bsp_max_candidates")->Int(1);
    int depth = cfg->FindArray("bsp_max_depth")->Int(1);
    float dirtol = cfg->FindArray("bsp_dir_tol")->Float(1);
    float postol = cfg->FindArray("bsp_pos_tol")->Float(1);
    SetBSPParams(postol, dirtol, depth, candidates, scale);
    DataRegisterFunc("set_bsp_params", SetBSPParams);
}

bool CheckBSPTree(const BSPNode *node, const Box &box) {
    if (!gBSPCheckScale)
        return true;
    Box box68;
    Multiply(box, gBSPCheckScale, box68);
    Hmx::Polygon polygon70;
    polygon70.points.resize(4);
    Transform tf50;
    polygon70.points[0] = Vector2(box68.mMin.x, box68.mMin.y);
    polygon70.points[1] = Vector2(box68.mMax.x, box68.mMin.y);
    polygon70.points[2] = Vector2(box68.mMax.x, box68.mMax.y);
    polygon70.points[3] = Vector2(box68.mMin.x, box68.mMax.y);
    tf50.m.Identity();
    tf50.v.Set(0, 0, box68.mMin.z);
    if (Intersect(tf50, polygon70, node))
        return false;
    // first intersect check

    polygon70.points.clear();
    polygon70.points.resize(4);
    polygon70.points[0] = Vector2(box68.mMin.x, -box68.mMax.y);
    polygon70.points[1] = Vector2(box68.mMax.x, -box68.mMax.y);
    polygon70.points[2] = Vector2(box68.mMax.x, -box68.mMin.y);
    polygon70.points[3] = Vector2(box68.mMin.x, -box68.mMin.y);
    float negone = -1.0f;
    tf50.m.Set(1.0f, 0.0f, 0.0f, 0.0f, negone, 0.0f, 0.0f, 0.0f, 0.0f);
    tf50.v.Set(0, 0, box68.mMax.z);
    if (Intersect(tf50, polygon70, node))
        return false;
    // second intersect check

    polygon70.points.clear();
    polygon70.points.resize(4);
    polygon70.points[0] = Vector2(box68.mMin.y, box68.mMin.z);
    polygon70.points[1] = Vector2(box68.mMax.y, box68.mMin.z);
    polygon70.points[2] = Vector2(box68.mMax.y, box68.mMax.z);
    polygon70.points[3] = Vector2(box68.mMin.y, box68.mMax.z);
    tf50.m.Set(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    tf50.v.Set(box68.mMin.x, 0, 0);
    if (Intersect(tf50, polygon70, node))
        return false;
    // third intersect check

    polygon70.points.clear();
    polygon70.points.resize(4);
    polygon70.points[0] = Vector2(-box68.mMax.y, box68.mMin.z);
    polygon70.points[1] = Vector2(-box68.mMin.y, box68.mMin.z);
    polygon70.points[2] = Vector2(-box68.mMin.y, box68.mMax.z);
    polygon70.points[3] = Vector2(-box68.mMax.y, box68.mMax.z);
    tf50.m.Set(1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    tf50.v.Set(box68.mMax.x, 0, 0);
    if (Intersect(tf50, polygon70, node))
        return false;
    // fourth intersect check

    polygon70.points.clear();
    polygon70.points.resize(4);
    polygon70.points[0] = Vector2(box68.mMin.x, box68.mMin.z);
    polygon70.points[1] = Vector2(box68.mMax.x, box68.mMin.z);
    polygon70.points[2] = Vector2(box68.mMax.x, box68.mMax.z);
    polygon70.points[3] = Vector2(box68.mMin.x, box68.mMax.z);
    tf50.m.Set(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    tf50.v.Set(0, box68.mMax.y, 0);
    if (Intersect(tf50, polygon70, node))
        return false;
    // fifth intersect check

    polygon70.points.clear();
    polygon70.points.resize(4);
    polygon70.points[0] = Vector2(-box68.mMax.x, box68.mMin.z);
    polygon70.points[1] = Vector2(-box68.mMin.x, box68.mMin.z);
    polygon70.points[2] = Vector2(-box68.mMin.x, box68.mMax.z);
    polygon70.points[3] = Vector2(-box68.mMax.x, box68.mMax.z);
    tf50.m.Set(-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    tf50.v.Set(0, box68.mMin.y, 0);
    if (Intersect(tf50, polygon70, node))
        return false;
    return true;
    // sixth and final intersect check
}

void MultiplyEq(BSPNode *n, const Transform &t) {
    for (; n != nullptr; n = n->right) {
        Multiply(n->plane, t, n->plane);
        Normalize(n->plane, n->plane);
        MultiplyEq(n->left, t);
    }
}
