#pragma once
#include "gesture/Skeleton.h"
#include "math/DoubleExponentialSmoother.h"
#include "math/Vec.h"

class HandInvokeGestureFilter {
public:
    HandInvokeGestureFilter();
    virtual ~HandInvokeGestureFilter();

    void Update(const Skeleton &, int ms);

private:
    float GetBend(const Vector3 &, const Vector3 &, const Vector3 &) const;

    Vector3DESmoother unk4; // 0x4
    Vector3 unk40; // 0x40
    Vector3DESmoother unk50; // 0x50
    Vector3DESmoother unk8c; // 0x8c
    Vector3DESmoother unkc8; // 0xc8
    Vector3DESmoother unk104; // 0x104
    bool unk140;
    int unk144;
};
