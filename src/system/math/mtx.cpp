#include "math/Mtx.h"
#include "math/Mtx.inl"

Hmx::Matrix2 Hmx::Matrix2::sID(Vector2(1, 0), Vector2(0, 1));
Hmx::Matrix3 Hmx::Matrix3::sID(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1));
Hmx::Matrix4 Hmx::Matrix4::sID(
    Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1)
);

Transform Transform::sID(Hmx::Matrix3::GetIdentity(), Vector3(0, 0, 0));

float Det(const Hmx::Matrix3 &m) {
    // M_{ij} where i,j are the left-out vecs
    // 1 indexed because lol linalg
    float m11 = ((m.y.y * m.z.z) - (m.z.y * m.y.z)) * m.x.x;
    float m12 = ((m.y.x * m.z.z) - (m.z.x * m.y.z)) * m.x.y;
    float m13 = ((m.y.x * m.z.y) - (m.z.x * m.y.y)) * m.x.z;
    float det = m11 - m12 + m13;
    if (det != 0) {
        det = 1.0f / det;
    }
    return det;
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

void Multiply(const Transform &l, const Transform &r, Transform &o) {
    if (&r != &o) {
        Multiply(l.v, r.m, o.v);
        Add(r.v, o.v, o.v);
    } else { // r is o! dangerous
        Vector3 v;
        // here be bullshit
        Multiply(l.v, r.m, v);
        Add(r.v, v, o.v);
    }
    Multiply(l.m, r.m, o.m);
}

float Det(const Hmx::Matrix4 &m4) {
    // M_{ij} where i,j are the left-out vecs
    // 1 indexed because lol linalg
    // clang-format off
    Hmx::Matrix3 m3 ( // M_{11}
         m4.m[1][1], m4.m[1][2], m4.m[1][3],
         m4.m[2][1], m4.m[2][2], m4.m[2][3],
         m4.m[3][1], m4.m[3][2], m4.m[3][3]
    );
    float det = Det(m3) * m4.m[0][0];
    // M_{12}
    m3.x.x = m4.m[1][0]; m3.x.y = m4.m[1][2]; m3.x.z = m4.m[1][3];
    m3.y.x = m4.m[2][0]; m3.y.y = m4.m[2][2]; m3.y.z = m4.m[2][3];
    m3.z.x = m4.m[3][0]; m3.z.y = m4.m[3][2]; m3.z.z = m4.m[3][3];
    det = -(Det(m3) * m4.m[0][1] - det);
    // M_{13}
    m3.x.x = m4.m[1][0]; m3.x.y = m4.m[1][1]; m3.x.z = m4.m[1][3];
    m3.y.x = m4.m[2][0]; m3.y.y = m4.m[2][1]; m3.y.z = m4.m[2][3];
    m3.z.x = m4.m[3][0]; m3.z.y = m4.m[3][1]; m3.z.z = m4.m[3][3];
    det += Det(m3) * m4.m[0][2];
    // M_{14}
    m3.x.x = m4.m[1][0]; m3.x.y = m4.m[1][1]; m3.x.z = m4.m[1][2];
    m3.y.x = m4.m[2][0]; m3.y.y = m4.m[2][1]; m3.y.z = m4.m[2][2];
    m3.z.x = m4.m[3][0]; m3.z.y = m4.m[3][1]; m3.z.z = m4.m[3][2];
    det = -(Det(m3) * m4.m[0][3] - det);
    // clang-format on
    return det;
}

void Invert(const Hmx::Matrix4 &reg, Hmx::Matrix4 &inv) {
    float det = Det(reg);
    if (fabsf(det) >= 0.0001) {
        det = 1 / det;
    } else {
        det = 0;
    }
    // i officially give up. go my ghidra
    float fVar9, fVar10, fVar11, fVar12, fVar13, fVar15, fVar16, fVar17, fVar18, fVar19,
        fVar20, fVar21, fVar22, dVar25, dVar26, dVar27, dVar28, dVar29, dVar30, dVar31,
        dVar32, dVar33, dVar34, dVar35, dVar36, dVar37, dVar38;
    fVar20 = reg.m[3].x * reg.m[2].y;
    fVar9 = reg.m[0].z;
    fVar10 = reg.m[1].x;
    fVar11 = reg.m[3].y;
    fVar12 = reg.m[3].z;
    fVar13 = reg.m[0].w;
    fVar22 = fVar11 * reg.m[2].x;
    fVar15 = reg.m[2].w;
    fVar16 = reg.m[3].x;
    fVar17 = reg.m[3].z;
    fVar18 = reg.m[2].x;
    fVar19 = reg.m[3].w;
    fVar21 = fVar18 * fVar17;
    fVar18 = fVar18 * fVar19;
    dVar38 = det;
    dVar37 =
        (-(fVar15 * reg.m[0].x * reg.m[1].y
           - (fVar15 * fVar10 * reg.m[0].y + reg.m[1].w * reg.m[0].x * reg.m[2].y
              + -(fVar13 * fVar10 * reg.m[2].y
                  - (fVar13 * reg.m[1].y * reg.m[2].x
                     - reg.m[1].w * reg.m[2].x * reg.m[0].y))))
         * dVar38);
    dVar34 =
        ((fVar19 * reg.m[0].x * reg.m[1].y
          + -(fVar19 * fVar10 * reg.m[0].y
              - -(reg.m[1].w * fVar11 * reg.m[0].x
                  - (fVar13 * fVar11 * fVar10
                     + (reg.m[1].w * reg.m[3].x * reg.m[0].y
                        - fVar13 * reg.m[3].x * reg.m[1].y)))))
         * dVar38);
    dVar32 =
        ((fVar15 * reg.m[0].x * reg.m[1].z
          + -(fVar15 * fVar10 * fVar9
              - -(reg.m[1].w * reg.m[2].z * reg.m[0].x
                  - (fVar13 * reg.m[2].z * fVar10
                     + (reg.m[1].w * reg.m[2].x * fVar9
                        - fVar13 * reg.m[1].z * reg.m[2].x)))))
         * dVar38);
    dVar31 =
        (-(reg.m[0].x * fVar19 * reg.m[2].y
           - (reg.m[0].y * fVar19 * reg.m[2].x + reg.m[0].x * fVar15 * fVar11
              + -(fVar13 * fVar22 - (fVar13 * fVar20 - reg.m[0].y * fVar15 * reg.m[3].x))))
         * dVar38);
    dVar35 = (reg.m[2].y * reg.m[1].z * fVar13);
    dVar33 =
        ((fVar10 * fVar19 * reg.m[2].y
          + -(reg.m[1].y * fVar19 * reg.m[2].x
              - -(fVar10 * fVar15 * fVar11
                  - (reg.m[1].w * fVar22
                     + (reg.m[1].y * fVar15 * reg.m[3].x - reg.m[1].w * fVar20)))))
         * dVar38);
    dVar30 =
        (-(reg.m[0].x * fVar19 * reg.m[1].z
           - (reg.m[0].x * fVar17 * reg.m[1].w + fVar19 * fVar10 * fVar9
              + -(fVar10 * fVar17 * fVar13
                  - (fVar13 * reg.m[1].z * fVar16 - fVar9 * reg.m[1].w * fVar16))))
         * dVar38);
    dVar29 =
        ((reg.m[2].z * fVar19 * reg.m[0].x
          + -(fVar18 * fVar9
              - -(fVar15 * fVar17 * reg.m[0].x
                  - (fVar21 * fVar13
                     + (fVar15 * fVar16 * fVar9 - reg.m[2].z * fVar16 * fVar13)))))
         * dVar38);
    dVar28 =
        (-(reg.m[2].z * fVar19 * fVar10
           - (fVar18 * reg.m[1].z + fVar15 * fVar17 * fVar10
              + -(fVar21 * reg.m[1].w
                  - (reg.m[2].z * fVar16 * reg.m[1].w - fVar15 * fVar16 * reg.m[1].z))))
         * dVar38);
    dVar26 = (reg.m[1][3] * reg.m[2][1]);
    dVar35 = -(dVar26 * reg.m[0][2] - dVar35);
    dVar26 = (reg.m[1][1] * reg.m[2][2]);
    dVar35 = -(dVar26 * reg.m[0][3] - dVar35);
    dVar26 = (reg.m[1][1] * reg.m[2][3]);
    dVar35 = (dVar26 * reg.m[0][2] + dVar35);
    dVar26 = (reg.m[1][3] * reg.m[2][2]);
    dVar35 = (reg.m[0][1] * dVar26 + dVar35);
    dVar26 = (reg.m[1][2] * reg.m[2][3]);
    dVar36 = (-(dVar26 * reg.m[0][1] - dVar35) * dVar38);
    dVar26 = (reg.m[1][3] * reg.m[3][1]);
    dVar35 = (dVar26 * reg.m[0][2]);
    dVar26 = (reg.m[1][2] * reg.m[3][1]);
    dVar35 = -(dVar26 * reg.m[0][3] - dVar35);
    dVar26 = (reg.m[1][1] * reg.m[3][2]);
    // pfVar23 = reg.m[0][3];
    // dVar35 = (*pfVar23 * dVar26 + dVar35);
    // pfVar23 = reg.m[3][2];
    // pfVar24 = reg.m[1][3];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][1];
    // dVar35 = -(dVar26 * *pfVar23 - dVar35);
    // pfVar23 = reg.m[3][3];
    // pfVar24 = reg.m[1][1];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][2];
    // dVar35 = -(dVar26 * *pfVar23 - dVar35);
    // pfVar23 = reg.m[3][3];
    // pfVar24 = reg.m[1][2];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][1];
    // dVar27 = ((dVar26 * *pfVar23 + dVar35) * dVar38);
    // pfVar23 = reg.m[3][1];
    // pfVar24 = reg.m[2][2];
    // dVar26 = (*pfVar23 * *pfVar24);
    // pfVar23 = reg.m[0][3];
    // dVar35 = (*pfVar23 * dVar26);
    // pfVar23 = reg.m[3][1];
    // pfVar24 = reg.m[2][3];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][2];
    // dVar35 = -(*pfVar23 * dVar26 - dVar35);
    // pfVar23 = reg.m[3][2];
    // pfVar24 = reg.m[2][1];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][3];
    // dVar35 = -(*pfVar23 * dVar26 - dVar35);
    // pfVar23 = reg.m[3][3];
    // pfVar24 = reg.m[2][1];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][2];
    // dVar35 = (*pfVar23 * dVar26 + dVar35);
    // pfVar23 = reg.m[3][2];
    // pfVar24 = reg.m[2][3];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][1];
    // dVar35 = (*pfVar23 * dVar26 + dVar35);
    // pfVar23 = reg.m[3][3];
    // pfVar24 = reg.m[2][2];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[0][1];
    // dVar25 = (-(*pfVar23 * dVar26 - dVar35) * dVar38);
    // pfVar23 = reg.m[3][1];
    // pfVar24 = reg.m[2][3];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[1][2];
    // dVar35 = (dVar26 * *pfVar23);
    // pfVar23 = reg.m[3][1];
    // pfVar24 = reg.m[2][2];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[1][3];
    // dVar35 = -(*pfVar23 * dVar26 - dVar35);
    // pfVar23 = reg.m[3][2];
    // pfVar24 = reg.m[2][1];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[1][3];
    // dVar35 = (*pfVar23 * dVar26 + dVar35);
    // pfVar23 = reg.m[3][2];
    // pfVar24 = reg.m[2][3];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[1][1];
    // dVar35 = -(*pfVar23 * dVar26 - dVar35);
    // pfVar23 = reg.m[3][3];
    // pfVar24 = reg.m[2][1];
    // dVar26 = (*pfVar24 * *pfVar23);
    // pfVar23 = reg.m[1][2];
    // dVar35 = -(*pfVar23 * dVar26 - dVar35);
    // pfVar23 = reg.m[3][3];
    // pfVar24 = reg.m[2][2];
    // dVar26 = (*pfVar24 * *pfVar23);
    inv.m[0].y = dVar25;
    inv.m[0].z = dVar27;
    inv.m[0].w = dVar36;
    inv.m[0].x = ((reg.m[1][1] * dVar26 + dVar35) * dVar38);
    inv.m[1].x = dVar28;
    inv.m[1].y = dVar29;
    inv.m[1].z = dVar30;
    inv.m[1].w = dVar32;
    inv.m[2].x = dVar33;
    inv.m[2].y = dVar31;
    inv.m[2].z = dVar34;
    inv.m[2].w = dVar37;
    inv.m[3].x =
        -(fVar12 * reg.m[2].y * fVar10
          - (fVar12 * reg.m[2].x * reg.m[1].y + fVar11 * reg.m[2].z * fVar10
             + -(fVar22 * reg.m[1].z
                 - (fVar20 * reg.m[1].z - reg.m[3].x * reg.m[2].z * reg.m[1].y))))
        * det;
    inv.m[3].y = (fVar12 * reg.m[2].y * reg.m[0].x
                  + -(fVar12 * reg.m[2].x * reg.m[0].y
                      - -(fVar11 * reg.m[2].z * reg.m[0].x
                          - (fVar22 * fVar9
                             + (reg.m[3].x * reg.m[2].z * reg.m[0].y - fVar20 * fVar9)))))
        * det;
    inv.m[3].z = -(fVar12 * reg.m[0].x * reg.m[1].y
                   - (fVar11 * reg.m[0].x * reg.m[1].z + fVar12 * fVar10 * reg.m[0].y
                      + -(fVar11 * fVar10 * fVar9
                          - (reg.m[3].x * reg.m[1].y * fVar9
                             - reg.m[3].x * reg.m[0].y * reg.m[1].z))))
        * det;
    inv.m[3].w = (reg.m[2].z * reg.m[0].x * reg.m[1].y
                  + -(reg.m[2].z * fVar10 * reg.m[0].y
                      - -(reg.m[0].x * reg.m[2].y * reg.m[1].z
                          - (fVar10 * reg.m[2].y * fVar9
                             + (reg.m[0].y * reg.m[1].z * reg.m[2].x
                                - fVar9 * reg.m[1].y * reg.m[2].x)))))
        * det;
}
