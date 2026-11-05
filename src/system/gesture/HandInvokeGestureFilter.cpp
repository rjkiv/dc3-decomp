#include "gesture/HandInvokeGestureFilter.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/DoubleExponentialSmoother.h"
#include "math/Vec.h"
#include "math/Vec.inl"
#include "stl/_cmath.h"
#include <cmath>

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
    float elapsed = elapsed_ms / 1000.0f;
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
            Clear();
        }
    }
}

bool HandInvokeGestureFilter::UpdateBodyPlane(Skeleton const &skeleton, float dt) {
    const TrackedJoint &rightShoulder = skeleton.ShoulderJoint(kSkeletonRight);
    const TrackedJoint &leftShoulder = skeleton.ShoulderJoint(kSkeletonLeft);

    bool ret = false;

    Vector3 vSubtracted;
    Subtract(leftShoulder.mPos[0], rightShoulder.mPos[0], vSubtracted);

    bool valid =
        skeleton.ShoulderJoint(kSkeletonLeft).mConfidence != kConfidenceNotTracked
        && skeleton.ShoulderJoint(kSkeletonRight).mConfidence != kConfidenceNotTracked
        && (vSubtracted.z * vSubtracted.z + vSubtracted.x * vSubtracted.x
            + vSubtracted.y * vSubtracted.y)
            > 0;

    if (valid) {
        Vector3 vForward;
        Cross(Vector3(0, 1, 0), vSubtracted, vForward);
        Normalize(vForward, vForward);
        mForward.Smooth(vForward, dt, true);

        Vector3 vFlattenedForward;
        Cross(Vector3(0, 1, 0), mForward.Value(), mRight);
        Normalize(mRight, mRight);

        vFlattenedForward = vForward;
        vFlattenedForward.y = 0;
        Normalize(vFlattenedForward, vFlattenedForward);

        if ((float)acos(
                (vFlattenedForward.x + vFlattenedForward.y) * 0 + vFlattenedForward.z
            )
            < 0.55850536f) {
            ret = true;
        }
    }
    return ret;
}

bool HandInvokeGestureFilter::CalcInPose(const Skeleton &skeleton, float dt) {
    const TrackedJoint &rightHand = skeleton.HandJoint(kSkeletonRight);
    mDampenedRightHand.Smooth(rightHand.mPos[0], dt, false);

    const TrackedJoint &rightShoulder = skeleton.ShoulderJoint(kSkeletonRight);
    mDampenedRightShoulder.Smooth(rightShoulder.mPos[0], dt, false);

    const TrackedJoint &leftHand = skeleton.HandJoint(kSkeletonLeft);
    mDampenedLeftHand.Smooth(leftHand.mPos[0], dt, false);

    const TrackedJoint &leftShoulder = skeleton.ShoulderJoint(kSkeletonLeft);
    mDampenedLeftShoulder.Smooth(leftShoulder.mPos[0], dt, false);

    Vector3 vSpine, vLArm, vRArm;

    Subtract(mDampenedRightHand.Value(), mDampenedRightShoulder.Value(), vRArm);
    Normalize(vRArm, vRArm);

    Subtract(mDampenedLeftShoulder.Value(), mDampenedLeftHand.Value(), vLArm);
    Normalize(vLArm, vLArm);

    return false;
}
