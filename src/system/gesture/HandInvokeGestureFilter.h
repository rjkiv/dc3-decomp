#pragma once
#include "gesture/Skeleton.h"
#include "math/DoubleExponentialSmoother.h"
#include "math/Vec.h"

class HandInvokeGestureFilter {
public:
    HandInvokeGestureFilter();
    virtual ~HandInvokeGestureFilter();

    void Update(const Skeleton &, int elapsed_ms);
    bool InPosition() const { return mInPosition; }

private:
    float GetBend(const Vector3 &, const Vector3 &, const Vector3 &) const;
    bool UpdateBodyPlane(const Skeleton &, float);
    bool CalcInPose(const Skeleton &, float);

    Vector3DESmoother mForward; // 0x4
    Vector3 mRight; // 0x40
    Vector3DESmoother mDampenedRightHand; // 0x50
    Vector3DESmoother mDampenedRightShoulder; // 0x8C
    Vector3DESmoother mDampenedLeftHand; // 0xC8
    Vector3DESmoother mDampenedLeftShoulder; // 0x104
    bool mInPosition; // 0x140
    int mTimer; // 0x144
};
