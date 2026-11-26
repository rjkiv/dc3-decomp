#include "math/Geo.h"
#include "Vec.h"
#include "math/Frustum.h"
#include "math/Mtx.h"
#include "math/Sphere.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/Std.h"

static float gBSPPosTol = 0.01f;
static float gBSPDirTol = 0.985f;
static int gBSPMaxDepth = 20;
static int gBSPMaxCandidates = 40;
static float gBSPCheckScale = 1.1f;
float gUnitsPerMeter = 39.370079f;

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
        NumNodes(node->front, num, maxDepth);
        NumNodes(node->back, num, maxDepth);
        num++;
        depth--;
    }
}

template <>
TextStream &operator<<(TextStream &ts, const std::vector<Vector2> &vec) {
    ts << "(size:" << vec.size() << ")";
    FOREACH_CONST (v2, vec) {
        ts << "\n" << v2 - vec.begin() << "\t" << *v2;
    }
    return ts;
}

BinStream &operator<<(BinStream &bs, const BSPNode *node) {
    if (node) {
        bs << true;
        bs << node->plane << node->front << node->back;
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
        bs >> node->plane >> node->front >> node->back;
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
    Vector3 v;
    Subtract(mMax, mMin, v);
    return v.x * v.y * v.z;
}

void Multiply(const Box &in, float scalar, Box &out) {
    Vector3 midpoint;
    Interp(in.mMin, in.mMax, 0.5f, midpoint);
    Subtract(in.mMax, midpoint, out.mMax);
    Scale(out.mMax, scalar, out.mMax);
    Add(out.mMax, midpoint, out.mMax);
    Subtract(in.mMin, midpoint, out.mMin);
    Scale(out.mMin, scalar, out.mMin);
    Add(out.mMin, midpoint, out.mMin);
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
    float a = in.a * in.a;
    float b = in.b * in.b;
    float c = in.c * in.c;
    float len = sqrtf(a + b + c);
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
        return;
    } else {
        float dot21 = Dot(diff21, diff21);
        if (f5 > dot21) {
            *vout = v2;
            return;
        } else {
            Scale(diff21, f5 / dot21, diff21);
            Add(v1, diff21, *vout);
            return;
        }
    }
}

void Plane::Set(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
    Vector3 tmp, cross;
    Subtract(v2, v1, tmp);
    Subtract(v3, v1, cross);
    Cross(tmp, cross, cross);
    Normalize(cross, cross);
    Set(v1, cross);
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
    SetBSPParams(
        cfg->FindArray("bsp_pos_tol")->Float(1),
        cfg->FindArray("bsp_dir_tol")->Float(1),
        cfg->FindArray("bsp_max_depth")->Int(1),
        cfg->FindArray("bsp_max_candidates")->Int(1),
        cfg->FindArray("bsp_check_scale")->Float(1)
    );
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
    tf50.m.Set(1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
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
    tf50.m.Set(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
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
    tf50.m.Set(0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f);
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
    tf50.m.Set(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f);
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
    tf50.m.Set(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    tf50.v.Set(0, box68.mMin.y, 0);
    if (Intersect(tf50, polygon70, node))
        return false;
    return true;
    // sixth and final intersect check
}

void BSPFace::Update() {
    MILO_ASSERT(p.points.size() > 2, 1730);
    // recalculate area
    area = 0.0f;
    const auto &points = p.points;
    for (auto p1 = points.begin(), p2 = p1 + 1, p3 = p1 + 2; p3 != points.end();
         p2 = p3++) {
        float x2y1 = p2->x * p1->y, y3x1 = p3->y * p1->x;
        float x3y2 = p3->x * p2->y;
        float f10 = p2->y * p1->x - x2y1;
        float f11 = p3->x * p1->y - y3x1;
        float f13 = p3->y * p2->x - x3y2;
        f13 += f10 + f11;
        area += f13 * 0.5f;
    }
    // recalculate planes
    planes.clear();
    Plane p;
    p.Set(t.v, t.m.z);
    planes.push_back(p);
    Vector3 basis(points.rbegin()->x, points.rbegin()->y, 0);
    Multiply(basis, t, basis);
    FOREACH_CONST (p, points)
        ;
}

void MultiplyEq(BSPNode *n, const Transform &t) {
    for (; n != nullptr; n = n->back) {
        Multiply(n->plane, t, n->plane);
        Normalize(n->plane, n->plane);
        MultiplyEq(n->front, t);
    }
}

void Intersect(const Hmx::Ray &s1, const Hmx::Ray &s2, Vector2 &v) {
    float base1X = s1.base.x;
    float base1Y = s1.base.y;
    float base2X = s2.base.x;
    float base2Y = s2.base.y;
    float dir1X = s1.dir.x;
    float dir2X = s2.dir.x;
    float dir1Y = s1.dir.y;
    float dir2Y = s2.dir.y;

    float cross = dir1Y * dir2X - dir2Y * dir1X;
    if (cross != 0) {
        float baseX = base1X - base2X;
        float baseY = base2Y - base1Y;
        cross = (baseX * dir1Y + baseY * dir1X) / cross;
        v.Set(cross * dir2X + base2X, cross * dir2Y + base2Y);
    } else {
        v = s1.base;
    }
}

void Intersect(const Transform &t, const Plane &p, Hmx::Ray &r) {
    float planeVecSquared = p.c * p.c + p.b * p.b + p.a * p.a;
    float scale = -(p.d / planeVecSquared);
    Vector3 scaledVec(p.a * scale, p.b * scale, p.c * scale);
    Vector3 point;
    MultiplyTranspose(scaledVec, t, point);
    const Vector3 &planeNormal = p.AsVector4().AsVector3();
    Vector3 normal;
    normal.Set(Dot(planeNormal, t.m.x), Dot(t.m.y, planeNormal), Dot(t.m.z, planeNormal));
    r.dir.Set(normal.x, normal.y);
    if (std::fabs(r.dir.y) > std::fabs(r.dir.x)) {
        r.base.Set(point.x, point.y + (normal.z / normal.y) * point.z);
    } else {
        r.base.Set(point.x + (normal.z / normal.x) * point.z, point.y);
    }
}

bool Intersect(const Segment &seg, const Triangle &tri, bool cull, float &dist) {
    const Vector3 &triFrameX = tri.frame.x;
    const Vector3 &triFrameY = tri.frame.y;
    Vector3 segDirection(
        seg.end.x - seg.start.x, seg.end.y - seg.start.y, seg.end.z - seg.start.z
    );
    const Vector3 &triFrameZ = tri.frame.z;
    float segDirDot = triFrameZ.x * segDirection.x + triFrameZ.y * segDirection.y
        + triFrameZ.z * segDirection.z;
    if (fabs(segDirDot) < 0.0001f || cull && segDirDot > 0.0f) {
        return false;
    }

    Vector3 vec3A(
        seg.start.z - tri.origin.z, seg.start.x - tri.origin.x, seg.start.y - tri.origin.y
    );
    float tempDot =
        -(triFrameZ.y * vec3A.z + (triFrameZ.z * vec3A.x + triFrameZ.x * vec3A.y));
    float t = tempDot / segDirDot;
    dist = t;
    if (t < 0.0f || t > 1.0f) {
        return false;
    }

    Vector3 scaled, hit, vec3B;
    Scale(segDirection, t, scaled);
    Add(seg.start, scaled, hit);
    Subtract(hit, tri.origin, vec3B);

    float dotXX =
        triFrameX.x * triFrameX.x + triFrameX.y * triFrameX.y + triFrameX.z * triFrameX.z;
    float dotYY =
        triFrameY.x * triFrameY.x + triFrameY.y * triFrameY.y + triFrameY.z * triFrameY.z;
    float dotXY =
        triFrameX.x * triFrameY.x + triFrameX.y * triFrameY.y + triFrameX.z * triFrameY.z;
    float dotX3B = triFrameX.x * vec3B.x + triFrameX.y * vec3B.y + triFrameX.z * vec3B.z;
    float dotY3B = triFrameY.x * vec3B.x + triFrameY.y * vec3B.y + triFrameY.z * vec3B.z;

    float inv = 1.0f / (dotXY * dotXY - dotYY * dotXX);
    float k = (dotY3B * dotXY - dotX3B * dotYY) * inv;
    if (k < 0.0f || k > 1.0f) {
        return false;
    }
    float j = (dotX3B * dotXY - dotY3B * dotXX) * inv;
    if (j < 0.0f || k + j > 1.0f) {
        return false;
    }
    return true;
}

bool Intersect(const Plane &plane, const Box &box) {
    Vector3 half(
        (box.mMax.x - box.mMin.x) * 0.5f,
        (box.mMax.y - box.mMin.y) * 0.5f,
        (box.mMax.z - box.mMin.z) * 0.5f
    );
    Vector3 centre;
    Add(box.mMin, half, centre);
    Vector3 vExtents(box.mMax.x - centre.x, box.mMax.y - centre.y, box.mMax.z - centre.z);

    Vector3 vMinVal, vMaxVal;
    const Vector3 &normal = plane.AsVector4().AsVector3();
    for (unsigned int i = 0; i < 3; i++) {
        if (normal[i] > 0.0f) {
            vMinVal[i] = -vExtents[i];
            vMaxVal[i] = vExtents[i];
        } else {
            vMinVal[i] = vExtents[i];
            vMaxVal[i] = -vExtents[i];
        }
    }
    if (plane.Dot(vMinVal) > 0.0f)
        return false;

    if (plane.Dot(vMaxVal) >= 0.0f)
        return true;

    return false;
}

bool Intersect(
    const Vector3 &start, const Vector3 &dir, const Box &box, float &tMin, float &tMax
) {
    tMin = 1.1920929e-07;
    tMax = 3.4028235e+38; // where tf do these numbers come from
    for (unsigned int i = 0; i < 3; i++) {
        float f1 = 1.0f / dir[i];
        float f2 = (box.mMin[i] - start[i]) * f1;
        float f3 = (box.mMax[i] - start[i]) * f1;
        if (f2 > f3) {
            float temp = f2;
            f2 = f3;
            f3 = temp;
        }
        tMin = Max(f2, tMin);
        tMax = Min(f3, tMax);
        if (tMin > tMax)
            return false;
    }
    return true;
}

bool Intersect(const Segment &seg, const Sphere &s) {
    float startZ = seg.start.z;
    float startX = seg.start.x;
    float startY = seg.start.y;
    float endZ = seg.end.z;
    float endX = seg.end.x;
    float endY = seg.end.y;
    float centreZ = s.center.z;
    float centreX = s.center.x;
    float centreY = s.center.y;
    Vector3 toCentre(centreX - startX, centreY - startY, centreZ - startZ);
    Vector3 dir;
    Subtract(seg.end, seg.start, dir);
    float dirSq = LengthSquared(dir);
    if (dirSq == 0.0f) {
        return false;
    }
    float t = (dir.y * toCentre.y + dir.z * toCentre.z + dir.x * toCentre.x) / dirSq;
    t = Clamp(0.0f, 1.0f, t);
    Interp(seg.start, seg.end, t, dir);
    Vector3 diff;
    Subtract(dir, s.center, diff);
    float rad = s.radius;
    if (LengthSquared(diff) > rad * rad) {
        return false;
    }
    return true;
}

void TriangleBoxProjectHelper(
    const Vector3 &axis,
    const Vector3 &v0,
    const Vector3 &v1,
    const Vector3 &v2,
    float &dot0,
    float &dot1,
    float &dot2
) {
    float z = axis.z;
    float p1 = z * v1.z;
    float p2 = z * v2.z;
    float p0 = z * v0.z;

    float x = axis.x;
    p1 += x * v1.x;
    p2 += x * v2.x;
    p0 += x * v0.x;

    float y = axis.y;
    p1 += y * v1.y;
    p2 += y * v2.y;
    p0 += y * v0.y;

    dot0 = p0;
    dot1 = p1;
    dot2 = p2;
}

bool Intersect(const Triangle &tri, const Box &box) {
    float halfX = (box.mMax.x - box.mMin.x) * 0.5f;
    float halfY = (box.mMax.y - box.mMin.y) * 0.5f;
    float halfZ = (box.mMax.z - box.mMin.z) * 0.5f;
    Vector3 vert1;
    vert1 = tri.origin;
    Vector3 vert2, vert3;
    Add(tri.origin, tri.frame.x, vert2);
    Add(tri.origin, tri.frame.y, vert3);
    Vector3 centre(box.mMin.x + halfX, box.mMin.y + halfY, box.mMin.z + halfZ);
    Vector3 vert3Vec;
    Subtract(vert3, centre, vert3Vec);
    Vector3 vert1Vec(vert1.x - centre.x, vert1.y - centre.y, vert1.z - centre.z);
    Vector3 vert2Vec(vert2.x - centre.x, vert2.y - centre.y, vert2.z - centre.z);
    float minX = Min(vert1Vec.x, vert2Vec.x);
    float maxX = Max(vert1Vec.x, vert2Vec.x);
    MinEq(minX, vert3Vec.x);
    MaxEq(maxX, vert3Vec.x);
    if (minX > halfX || maxX < -halfX) {
        return false;
    }
    float minY = Min(vert1Vec.y, vert2Vec.y);
    float maxY = Max(vert1Vec.y, vert2Vec.y);
    MinEq(minY, vert3Vec.y);
    MaxEq(maxY, vert3Vec.y);
    if (minY > halfY || maxY < -halfY) {
        return false;
    }
    float minZ = Min(vert1Vec.z, vert2Vec.z);
    float maxZ = Max(vert1Vec.z, vert2Vec.z);
    MinEq(minZ, vert3Vec.z);
    MaxEq(maxZ, vert3Vec.z);
    if (minZ > halfZ || maxZ < -halfZ) {
        return false;
    }
    Plane triPlane(vert1Vec, tri.frame.z);
    if (!Intersect(triPlane, box)) {
        return false;
    }
    Vector3 edge0(
        vert2Vec.x - vert1Vec.x, vert2Vec.y - vert1Vec.y, vert2Vec.z - vert1Vec.z
    );
    Vector3 edge1(
        vert3Vec.x - vert2Vec.x, vert3Vec.y - vert2Vec.y, vert3Vec.z - vert2Vec.z
    );
    Vector3 edge2(
        vert1Vec.x - vert3Vec.x, vert1Vec.y - vert3Vec.y, vert1Vec.z - vert3Vec.z
    );
    Vector3 unkVec1[9] = {
        Vector3(0.0f, -edge0.z, edge0.y), Vector3(0.0f, -edge1.z, edge1.y),
        Vector3(0.0f, -edge2.z, edge2.y), Vector3(edge0.z, 0.0f, -edge0.x),
        Vector3(edge1.z, 0.0f, -edge1.x), Vector3(edge2.z, 0.0f, -edge2.x),
        Vector3(-edge0.y, edge0.x, 0.0f), Vector3(-edge1.y, edge1.x, 0.0f),
        Vector3(-edge2.y, edge2.x, 0.0f),
    };
    float unkFloatArr[9];
    for (unsigned int i = 0; i < 9; i++) {
        float f1 = unkVec1[i][0];
        if (f1 <= 0.0f) {
            f1 = -f1;
        }
        float f2 = unkVec1[i][1];
        if (f2 <= 0.0f) {
            f2 = -f2;
        }
        float projZ = unkVec1[i][2];
        if (projZ <= 0.0f) {
            projZ = -projZ;
        }
        unkFloatArr[i] = f2 * halfY + projZ * halfZ + f1 * halfX;
        float dot0, dot1, dot2;
        TriangleBoxProjectHelper(
            unkVec1[i], vert1Vec, vert2Vec, vert3Vec, dot0, dot1, dot2
        );
        float maxDot = Max(dot0, Max(dot1, dot2));
        float minDot = Min(dot0, Min(dot1, dot2));
        if (maxDot < -unkFloatArr[i] || minDot > unkFloatArr[i]) {
            return false;
        }
    }
    return true;
}

void Frustum::Set(float nearDist, float farDist, float yFov, float yRatio) {
    front.Set(0, 1, 0, -nearDist);
    back.Set(0, -1, 0, farDist);
    float sinFov = sinf(yFov / 2);
    float cosFov = cosf(yFov / 2);
    float sinRatio = sinFov / yRatio;
    top.Set(0, sinFov, -cosFov, 0);
    bottom.Set(0, sinFov, cosFov, 0);
    float len = sqrtf(sinRatio * sinRatio + cosFov * cosFov);
    float f1;
    if (len != 0) {
        f1 = 1 / len;
    } else {
        f1 = 0;
    }
    left.Set(f1 * cosFov, f1 * sinRatio, 0, 0);
    right.Set(-(f1 * cosFov), f1 * sinRatio, 0, 0);
    if (yFov == 0) {
        right.d = 1;
        left.d = 1;
        top.d = yRatio;
        bottom.d = yRatio;
    }
}

void BSPFace::Set(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3) {
    Subtract(p2, p1, t.m.x);
    Normalize(t.m.x, t.m.x);
    Subtract(p3, p1, t.m.y);

    Cross(t.m.x, t.m.y, t.m.z);
    Normalize(t.m.z, t.m.z);
    Cross(t.m.z, t.m.x, t.m.y);

    t.v = p1;
    p.points.clear();

    Vector3 v;
    MultiplyTranspose(p1, t, v);
    p.points.push_back(Vector2(v.x, v.y));
    MultiplyTranspose(p2, t, v);
    p.points.push_back(Vector2(v.x, v.y));
    MultiplyTranspose(p3, t, v);
    p.points.push_back(Vector2(v.x, v.y));
    Update();
}

void Multiply(const Plane &a, const Transform &b, Plane &c) {
    Hmx::Matrix3 m;
    FastInvert(b.m, m);
    Vector3 vmult;
    Multiply(m, a.AsVector4().AsVector3(), vmult);
    Vector3 on;
    Multiply(a.On(), b, on);
    c.Set(on, vmult);
}
