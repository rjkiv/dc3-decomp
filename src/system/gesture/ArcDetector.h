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

    static float _swipeRetentionFactor;
    static float _acceptablePathErrorRatio;

    SkeletonSide mSide; // 0x4
    SkeletonJoint unk8; // 0x8
    SkeletonJoint unkc; // 0xc
    std::list<Vector3> mJointPath; // 0x10
    Vector3 unk18;
    float unk28;
    float unk2c;
    float unk30;
    bool unk34;
    bool unk35;
    float unk38;
    int unk3c;
    Vector3 unk40;
};
