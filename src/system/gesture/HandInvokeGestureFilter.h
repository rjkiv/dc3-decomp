#pragma once

#include "gesture/Skeleton.h"
#include "math/Vec.h"
class HandInvokeGestureFilter {
public:
    HandInvokeGestureFilter();
    ~HandInvokeGestureFilter();

    void Update(const Skeleton &, int ms);

private:
    float GetBend(const Vector3 &, const Vector3 &, const Vector3 &) const;
};
