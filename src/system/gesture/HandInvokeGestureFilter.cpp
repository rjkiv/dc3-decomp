#include "gesture/HandInvokeGestureFilter.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "stl/_cmath.h"

HandInvokeGestureFilter::HandInvokeGestureFilter()
    : mForward(Vector3(0, 0, -1), 3, 0), mDampenedRightHand(Vector3::GetZero(), 3, 0),
      mDampenedRightShoulder(Vector3::GetZero(), 6, 0),
      mDampenedLeftHand(Vector3::GetZero(), 3, 0),
      mDampenedLeftShoulder(Vector3::GetZero(), 6, 0), mInPosition(0), mTimer(0) {}

HandInvokeGestureFilter::~HandInvokeGestureFilter() {}

float HandInvokeGestureFilter::GetBend(
    const Vector3 &vHand, const Vector3 &vElbow, const Vector3 &vShoulder
) const {
    Vector3 vShoulderToElbow(
        vElbow.x - vShoulder.x, vElbow.y - vShoulder.y, vElbow.z - vShoulder.z
    );
    Normalize(vShoulderToElbow, vShoulderToElbow);
    Vector3 vShoulderToHand(
        vHand.x - vShoulder.x, vHand.y - vShoulder.y, vHand.z - vShoulder.z
    );
    Normalize(vShoulderToHand, vShoulderToHand);
    return std::acos(
        vShoulderToElbow.y * vShoulderToHand.y + vShoulderToElbow.z * vShoulderToHand.z
        + vShoulderToElbow.x * vShoulderToHand.x
    );
}

void HandInvokeGestureFilter::Update(const Skeleton &skeleton, int elapsed_ms) {
    float elapsed = elapsed_ms * 0.001f;
    bool wasInPosition = mInPosition;

    if (skeleton.IsTracked()) {
        bool updated = UpdateBodyPlane(skeleton, elapsed);
        mInPosition = false;

        if (updated) {
            mInPosition = CalcInPose(skeleton, elapsed);

            if (mInPosition)
                return;
        }

        if (!wasInPosition) {
            mInPosition = false;
            mTimer = 0;
        }
    }
}
