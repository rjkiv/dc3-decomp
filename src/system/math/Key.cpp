#include "math/Key.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "math/Rot.h"

void InterpTangent(
    const Vector3 &va,
    const Vector3 &ta,
    const Vector3 &vb,
    const Vector3 &tb,
    float r,
    Vector3 &t
) {
    float ftimes6 = r * 6.0f;
    float rsq = r * r;
    float scale3 = rsq * 3.0f;

    Scale(va, rsq * 6 - ftimes6, t);
    Vector3 vtmp;
    Scale(ta, 1.0f + (scale3 - r * 4), vtmp);
    Add(t, vtmp, t);
    Scale(vb, ftimes6 - rsq * 6, vtmp);
    Add(t, vtmp, t);
    Scale(tb, scale3 - r * 2, vtmp);
    Add(t, vtmp, t);
}

void SplineTangent(const Keys<Vector3, Vector3> &keys, int i, Vector3 &r) {
    int size = keys.size();
    MILO_ASSERT(size > 1, 0x17);
    if (size == 2) {
        Subtract(keys[1].value, keys[0].value, r);
    } else if (i <= 0) {
        Subtract(keys[1].value, keys[0].value, r);
        Scale(r, 1.5f, r);
        Vector3 vtmp;
        Subtract(keys[2].value, keys[0].value, vtmp);
        Scale(vtmp, 0.25f, vtmp);
        Subtract(r, vtmp, r);
    } else if (i >= size - 1) {
        Subtract(keys[size - 1].value, keys[size - 2].value, r);
        Scale(r, 1.5f, r);
        Vector3 vtmp;
        Subtract(keys[size - 1].value, keys[size - 3].value, vtmp);
        Scale(vtmp, 0.25f, vtmp);
        Subtract(r, vtmp, r);
    } else {
        Subtract(keys[i + 1].value, keys[i - 1].value, r);
        Scale(r, 0.5f, r);
    }
}

void InterpVector(
    const Keys<Vector3, Vector3> &keys,
    const Key<Vector3> *a,
    const Key<Vector3> *b,
    float t,
    bool spline,
    Vector3 &r,
    Vector3 *tangent
) {
    if (keys.size() < 3) {
        spline = false;
        if (keys.size() < 2) {
            if (tangent)
                tangent->Set(0.0f, 1.0f, 0.0f);
            if (keys.size() != 0)
                r = a->value;
            else
                r.Set(0, 0, 0);
            return;
        }
    }
    int idx = a - keys.begin();
    if (spline) {
        float fsq = t * t;
        float fcubed = fsq * t;
        float fsq3 = fsq * 3.0f;
        Scale(a->value, (fcubed * 2.0f - fsq3) + 1.0f, r);
        Vector3 v70;
        SplineTangent(keys, idx, v70);
        Vector3 v7c;
        Vector3 v88;
        Scale(v70, t + fsq * -2.0f + fcubed, v88);
        Add(r, v88, r);
        Scale(b->value, fcubed * -2.0f + fsq3, v88);
        Add(r, v88, r);
        SplineTangent(keys, idx + 1, v7c);
        Scale(v7c, fcubed - fsq, v88);
        Add(r, v88, r);
        if (tangent) {
            InterpTangent(a->value, v70, b->value, v7c, t, *tangent);
        }
    } else {
        Interp(a->value, b->value, t, r);
        if (tangent) {
            if (idx == keys.size() - 1) {
                idx--;
            }
            Subtract(keys[idx + 1].value, keys[idx].value, *tangent);
        }
    }
}

void InterpVector(
    const Keys<Vector3, Vector3> &keys,
    bool spline,
    float frame,
    Vector3 &r,
    Vector3 *tangent
) {
    const Key<Vector3> *prev;
    const Key<Vector3> *next;
    float ref;
    keys.AtFrame(frame, prev, next, ref);
    InterpVector(keys, prev, next, ref, spline, r, tangent);
}

void QuatSpline(
    const Keys<Hmx::Quat, Hmx::Quat> &keys,
    const Key<Hmx::Quat> *a,
    const Key<Hmx::Quat> *b,
    float t,
    Hmx::Quat &ret
) {
    MILO_ASSERT(keys.size(), 0x9B);
    if (a == b) {
        ret = a->value;
    } else {
        float tsq = t * t;
        float t3 = tsq * t;
        int idx = a - &keys.front();
        int nextIdx = idx + 1;
        Hmx::Quat q[4];
        q[1] = a->value;
        q[2] = b->value;
        q[0] = idx == 0 ? q[1] : keys[idx - 1].value;
        q[3] = nextIdx == keys.size() - 1 ? q[2] : keys[nextIdx + 1].value;
        NormalizeTo(q[1], q[0]);
        NormalizeTo(q[1], q[2]);
        NormalizeTo(q[1], q[3]);
        for (int i = 0; i < 4; i++) {
            float q3 = q[3][i];
            float q2 = q[2][i];
            float q1 = q[1][i];
            float q0 = q[0][i];
            ret[i] = (t3 * (q3 + -(q2 * 3 - (q1 * 3 - q0)))
                      + (tsq * ((q2 * 4 + (q0 * 2 - q1 * 5)) - q3)
                         + (q1 * 2 + (t * (q2 - q0)))))
                / 2;
        }
        Normalize(ret, ret);
    }
}
