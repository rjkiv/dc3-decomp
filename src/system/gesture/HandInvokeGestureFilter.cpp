#include "gesture/HandInvokeGestureFilter.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "stl/_cmath.h"

HandInvokeGestureFilter::HandInvokeGestureFilter()
    : unk4(Vector3(0, 0, -1), 3, 0), unk50(Vector3::GetZero(), 3, 0),
      unk8c(Vector3::GetZero(), 6, 0), unkc8(Vector3::GetZero(), 3, 0),
      unk104(Vector3::GetZero(), 6, 0), unk140(0), unk144(0) {}

HandInvokeGestureFilter::~HandInvokeGestureFilter() {}

float HandInvokeGestureFilter::GetBend(
    const Vector3 &vec1, const Vector3 &vec2, const Vector3 &vec3
) const {
    Vector3 v1(vec2.x - vec3.x, vec2.y - vec3.y, vec2.z - vec3.z);
    Normalize(v1, v1);
    Vector3 v2(vec1.x - vec3.x, vec1.y - vec3.y, vec1.z - vec3.z);
    Normalize(v2, v2);
    return std::acos(v1.y * v2.y + v1.z * v2.z + v1.x * v2.x);
}
