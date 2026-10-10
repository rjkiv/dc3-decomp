#pragma once
#include "gesture/BaseSkeleton.h"
#include "math/DoubleExponentialSmoother.h"
#include "math/Plane.h"
#include "xdk/NUI.h"

// TrackedJoint size: 0x74
struct TrackedJoint {
    Vector3 mPos[kNumCoordSys]; // 0x0
    Vector3 mRawCameraPos; // 0x60
    JointConfidence mConfidence; // 0x70
};

struct SkeletonFrame;
class ArchiveSkeleton;
class CameraInput;

enum SkeletonTracking {
    /** "Not Tracked" */
    kSkeletonNotTracked = 0,
    /** "Position Based Tracking (Blob)" */
    kSkeletonPositionOnly = 1,
    /** "Full Skeleton Tracking" */
    kSkeletonTracked = 2
};

// size: 0xAD4
class Skeleton : public BaseSkeleton {
public:
    Skeleton();
    virtual void JointPos(SkeletonCoordSys, SkeletonJoint, Vector3 &) const; // 0x4
    virtual bool Displacement(
        const SkeletonHistory *, SkeletonCoordSys, SkeletonJoint, int, Vector3 &, int &
    ) const; // 0x8
    virtual bool Displacements(
        const SkeletonHistory *, SkeletonCoordSys, int, Vector3 *, int &
    ) const; // 0xc
    virtual JointConfidence JointConf(SkeletonJoint) const; // 0x10
    virtual bool IsTracked() const; // 0x14
    virtual int QualityFlags() const; // 0x18
    virtual int ElapsedMs() const; // 0x1c
    virtual void CameraToPlayerXfm(SkeletonCoordSys, Transform &) const; // 0x20
    virtual void CamJointPositions(Vector3 *) const; // 0x28
    virtual void CamBoneLengths(float *) const; // 0x2c
    virtual float BoneLength(SkeletonBone, SkeletonCoordSys) const; // 0x30

    Skeleton &operator=(const Skeleton &);
    void PostUpdate();
    bool IsValid() const;
    bool IsSitting() const;
    bool IsSideways() const;
    const TrackedJoint &HandJoint(SkeletonSide) const;
    const TrackedJoint &ElbowJoint(SkeletonSide) const;
    const TrackedJoint &ShoulderJoint(SkeletonSide) const;
    const TrackedJoint &HipJoint(SkeletonSide) const;
    const TrackedJoint &KneeJoint(SkeletonSide) const;
    bool ProfileMatched() const;
    int GetEnrollmentIndex() const;
    bool NeedIdentify() const;
    void ScreenPos(SkeletonJoint, Vector2 &) const;
    bool Velocity(
        const SkeletonHistory &, SkeletonCoordSys, SkeletonJoint, int, Vector3 &, int &
    ) const;
    bool RequestIdentity();
    bool EnrollIdentity(int);
    void Init();
    void Poll(int, const SkeletonFrame &);
    const TrackedJoint *TrackedJoints() const { return mTrackedJoints; }
    int TrackingID() const { return mTrackingID; }
    int SkeletonIndex() const { return mSkeletonIndex; }
    SkeletonTracking TrackingState() const { return mTracking; }
    const Vector3 &Center() const { return mCenter; }

    static int IdentityCallback(void *, NUI_IDENTITY_MESSAGE *);

protected:
    // size 0x148
    struct CameraDisplacement {
        int mCachedMs; // 0x0
        int mActualMs; // 0x4
        Vector3 mCameraDisplacements[kNumJoints]; // 0x8
    };

    bool
    PrevTrackedSkeleton(const SkeletonHistory *, int, int &, ArchiveSkeleton &) const;

    TrackedJoint mTrackedJoints[kNumJoints]; // 0x4
    float mCameraBoneLengths[kNumBones]; // 0x914
    Transform mCameraToPlayer[5]; // 0x960
    SkeletonTracking mTracking; // 0xaa0
    int mQualityFlags; // 0xaa4
    int mElapsedMs; // 0xaa8
    int mTrackingID; // 0xaac
    Vector3 mCenter; // 0xab0
    int mSkeletonIndex; // 0xac0
    float mPelvisHeight; // 0xac4
    std::vector<CameraDisplacement> mCachedCameraDisplacements; // 0xac8
};

class SkeletonCallback {
public:
    virtual ~SkeletonCallback() {}
    virtual void Clear() = 0;
    virtual void Update(const struct SkeletonUpdateData &) = 0;
    virtual void PostUpdate(const struct SkeletonUpdateData *) = 0;
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) = 0;
};

// size 0x2f0
struct SkeletonData {
    SkeletonTracking mTracking; // 0x0
    Vector3 mRawJointPositions[kNumJoints]; // 0x4
    Vector3 mJointPositions[kNumJoints]; // 0x144
    JointConfidence mJointConfidences[kNumJoints]; // 0x284
    int mQualityFlags; // 0x2d4
    int mTrackingID; // 0x2d8
    int mEnrollmentIndex; // 0x2dc
    Vector3 mCenter; // 0x2e0
};

struct SkeletonUpdateData {
    SkeletonUpdateData(
        const Skeleton *(&players)[2],
        const Skeleton *(&all)[6],
        const SkeletonFrame &frame,
        const SkeletonHistory &history,
        const CameraInput &camInput
    )
        : mPlayerSkeletons(players), mAllSkeletons(all), mFrame(frame), mHistory(history),
          mCamInput(camInput) {}
    const Skeleton *(&mPlayerSkeletons)[2]; // 0x0
    const Skeleton *(&mAllSkeletons)[6]; // 0x4
    const SkeletonFrame &mFrame; // 0x8
    const SkeletonHistory &mHistory; // 0xc
    const CameraInput &mCamInput; // 0x10
};

// size 0x11c8
struct SkeletonFrame {
    void Create(const NUI_SKELETON_FRAME &, int);
    float TiltAngle() const;
    static void Init();

    static Vector3DESmoother sUpVectorSmoother;

    int mFrameNumber; // 0x0
    int mElapsedMs; // 0x4
    Vector3 mUpVector; // 0x8
    Plane mFloorPlane; // 0x18
    SkeletonData mSkeletonDatas[6]; // 0x28
};
