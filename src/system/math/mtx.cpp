#include "math/Mtx.h"

Hmx::Matrix2 Hmx::Matrix2::sID(Vector2(1, 0), Vector2(0, 1));
Hmx::Matrix3 Hmx::Matrix3::sID(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1));
Hmx::Matrix4 Hmx::Matrix4::sID(
    Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1)
);

Transform Transform::sID(Hmx::Matrix3::GetIdentity(), Vector3(0, 0, 0));

float Det(const Hmx::Matrix3 &m) {
    Vector3 cross;
    Cross(m.z, m.y, cross);
    float det = Dot(m.x, cross);
    if (det != 0) {
        det = 1.0f / det;
    }
    return det;
}

void Invert(const Hmx::Matrix3 &min, Hmx::Matrix3 &mout) {
    float mult = 0;
    float f1 = (min.y.x - min.z.y - min.z.x * min.y.y) + min.x.z
        + ((min.y.y * min.z.z - min.y.z * min.z.y) * min.x.x
           - (min.y.x * min.z.z - min.z.x * min.y.z) * min.x.y);
    if (f1 != 0) {
        mult = 1.0f / f1;
    }
    mout.Set(
        (min.z.z * min.y.y - min.y.z * min.z.y) * mult,
        -((min.z.z * min.x.y - min.x.z * min.z.y) * mult),
        (min.y.z * min.x.y - min.x.z * min.y.y) * mult,
        -((min.z.z * min.y.x - min.y.z * min.z.x) * mult),
        (min.z.z * min.x.x - min.x.z * min.z.x) * mult,
        -((min.y.z * min.x.x - min.x.z * min.y.x) * mult),
        (min.z.y * min.y.x - min.z.x * min.y.y) * mult,
        -((min.z.y * min.y.x - min.z.x * min.x.y) * mult),
        (min.y.y * min.x.x - min.x.y * min.y.x) * mult
    );
}

void FastInvert(const Hmx::Matrix3 &min, Hmx::Matrix3 &mout) {
    float xdot = Dot(min.x, min.x);
    if (xdot != 0)
        xdot = 1.0f / xdot;
    float ydot = Dot(min.y, min.y);
    if (ydot != 0)
        ydot = 1.0f / ydot;
    float zdot = Dot(min.z, min.z);
    if (zdot != 0)
        zdot = 1.0f / zdot;
    mout.Set(
        min.x.x * xdot,
        min.y.x * ydot,
        min.z.x * zdot,
        min.x.y * xdot,
        min.y.y * ydot,
        min.z.y * zdot,
        min.x.z * xdot,
        min.y.z * ydot,
        min.z.z * zdot
    );
}
