#pragma once
#include "gesture/BaseSkeleton.h"
#include "math/DoubleExponentialSmoother.h"
#include "xdk/NUI.h"

// TrackedJoint size: 0x74
struct TrackedJoint {
    Vector3 mJointPos[kNumCoordSys]; // 0x0
    Vector3 unk60;
    JointConfidence mJointConf; // 0x70
};

struct SkeletonFrame;
class ArchiveSkeleton;

enum SkeletonTrackingState {
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
    virtual bool
    Displacement(const SkeletonHistory *, SkeletonCoordSys, SkeletonJoint, int, Vector3 &, int &)
        const; // 0x8
    virtual bool
    Displacements(const SkeletonHistory *, SkeletonCoordSys, int, Vector3 *, int &)
        const; // 0xc
    virtual JointConfidence JointConf(SkeletonJoint) const; // 0x10
    virtual bool IsTracked() const; // 0x14
    virtual int QualityFlags() const; // 0x18
    virtual int ElapsedMs() const; // 0x1c
    virtual void CameraToPlayerXfm(SkeletonCoordSys, Transform &) const; // 0x20
    virtual void CamJointPositions(Vector3 *) const; // 0x28
    virtual void CamBoneLengths(float *) const; // 0x2c
    virtual float BoneLength(SkeletonBone, SkeletonCoordSys) const; // 0x30

    const TrackedJoint *TrackedJoints() const { return mTrackedJoints; }
    int TrackingID() const { return mTrackingID; }
    int SkeletonIndex() const { return mSkeletonIdx; }
    SkeletonTrackingState TrackingState() const { return mTracking; }
    Skeleton &operator=(const Skeleton &);
    void PostUpdate() {}
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
    bool
    Velocity(const SkeletonHistory &, SkeletonCoordSys, SkeletonJoint, int, Vector3 &, int &)
        const;
    bool RequestIdentity();
    bool EnrollIdentity(int);
    void Init();
    void Poll(int, const SkeletonFrame &);

    // static int IdentityCallback(void*, _NUI_IDENTITY_MESSAGE*);

protected:
    // size 0x148
    struct CameraDisplacement {
        int unk0;
        int unk4;
        Vector3 unk8[kNumJoints];
    };

    bool
    PrevTrackedSkeleton(const SkeletonHistory *, int, int &, ArchiveSkeleton &) const;

    TrackedJoint mTrackedJoints[kNumJoints]; // 0x4
    float mCamBoneLengths[kNumBones]; // 0x914
    Transform mPlayerXfms[5]; // 0x960
    SkeletonTrackingState mTracking; // 0xaa0
    int mQualityFlags; // 0xaa4
    int mElapsedMs; // 0xaa8
    int mTrackingID; // 0xaac
    Vector3 unkab0;
    int mSkeletonIdx; // 0xac0
    float unkac4;
    std::vector<CameraDisplacement> mCamDisplacements; // 0xac8
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
    SkeletonTrackingState mTracking; // 0x0
    Vector3 unk4[kNumJoints]; // 0x4
    Vector3 unk144[kNumJoints]; // 0x144
    int unk284[kNumJoints]; // 0x284
    int mQualityFlags; // 0x2d4
    int mTrackingID; // 0x2d8
    int unk2dc;
    Vector3 unk2e0;
};

// size 0x11c8
struct SkeletonFrame {
    void Create(const NUI_SKELETON_FRAME &, int);
    float TiltAngle() const;
    static void Init();

    static Vector3DESmoother sUpVectorSmoother;

    int unk0; // 0x0 - frame number?
    int mElapsedMs; // 0x4
    Vector3 unk8; // 0x8 - camera angle?
    Vector4 unk18; // 0x18 - maybe Vector4, maybe XMVECTOR, idk
    SkeletonData mSkeletonDatas[6]; // 0x28
};
