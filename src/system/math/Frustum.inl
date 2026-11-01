#pragma once
#include "Frustum.h"
#include "Mtx.h"
#include "Sphere.h"

bool operator>(const Sphere &s, const Frustum &f) {
    return s < f.front || s < f.back || s < f.left || s < f.right || s < f.top
        || s < f.bottom;
}

inline void Multiply(const Frustum &fin, const Transform &tf, Frustum &fout) {
    Multiply(fin.front, tf, fout.front);
    Multiply(fin.back, tf, fout.back);
    Multiply(fin.left, tf, fout.left);
    Multiply(fin.right, tf, fout.right);
    Multiply(fin.top, tf, fout.top);
    Multiply(fin.bottom, tf, fout.bottom);
}
