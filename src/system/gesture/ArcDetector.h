#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "rndobj/Overlay.h"

class ArcDetector {
public:
    ArcDetector();
    virtual ~ArcDetector();

    void Clear();
    void Initialize(SkeletonSide, SkeletonJoint, SkeletonJoint, float);
    void PrintJointPath() const;
    float GetSwipeAmount() const;
    void ResetHoverTimer();
    bool IsLockedIn() const;
    float UpdateOverlay(RndOverlay *, float);
    void Update(const Skeleton &, int);
    void Draw(const Skeleton &, SkeletonViz &);
    int NumJointsInPath() const { return mJointPath.size(); }

private:
    Vector3 GetCurveStart() const;
    void TryToStartSwipe(const Vector3 &, const Skeleton &);
    float GetPathLength() const;
    float GetPathError() const;
    void SwipeFailed(const Skeleton &);
    void CullPath();
    void DrawPath(std::list<Vector3>, SkeletonViz &, Hmx::Color, const Vector3 &) const;
    bool IsPathAcceptable() const;

    SkeletonSide mDirection; // 0x4
    SkeletonJoint mTrackJoint; // 0x8
    SkeletonJoint mRefJoint; // 0xc
    std::list<Vector3> mJointPath; // 0x10
    Vector3 mCurveStart; // 0x18
    float mArcSize; // 0x28
    float mArcHeight; // 0x2c
    float mDesiredLength; // 0x30
    bool mInitialized; // 0x34
    bool mNeedSwipeReset; // 0x35
    float mMaxSwipePercent; // 0x38
    int mHoverTimer; // 0x3c
    Vector3 mLastRelativeSwipePos; // 0x40
};
