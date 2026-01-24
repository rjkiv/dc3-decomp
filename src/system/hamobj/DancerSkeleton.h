#pragma once
#include "gesture/BaseSkeleton.h"
#include "math/Vec.h"
#include "utl/BinStream.h"

class DancerSkeleton : public BaseSkeleton {
public:
    DancerSkeleton();
    virtual ~DancerSkeleton() {} // 0x0
    virtual void JointPos(SkeletonCoordSys, SkeletonJoint, Vector3 &) const; // 0x4
    virtual bool Displacement(
        const SkeletonHistory *, SkeletonCoordSys, SkeletonJoint, int, Vector3 &, int &
    ) const; // 0x8
    virtual bool Displacements(
        const SkeletonHistory *, SkeletonCoordSys, int, Vector3 *, int &
    ) const; // 0xc
    virtual JointConfidence JointConf(SkeletonJoint) const {
        return kConfidenceTracked;
    } // 0x10
    virtual bool IsTracked() const { return mTracked != 0; } // 0x14
    virtual int QualityFlags() const { return 0; } // 0x18
    virtual int ElapsedMs() const { return mElapsedMs; } // 0x1c
    virtual void CameraToPlayerXfm(SkeletonCoordSys, Transform &) const; // 0x20
    virtual void CamJointPositions(Vector3 *) const; // 0x28
    virtual void CamBoneLengths(float *) const; // 0x2c
    virtual float BoneLength(SkeletonBone, SkeletonCoordSys) const; // 0x30

    void Read(BinStream &);
    void Write(BinStream &);
    void CamJointDisplacements(Vector3 *) const;
    void Init();
    void Set(const BaseSkeleton &);
    void SetDisplacementElapsedMs(int);
    const Vector3 &CamJointPos(SkeletonJoint) const;
    const Vector3 &CamJointDisplacement(SkeletonJoint) const;
    void SetCamJointPos(SkeletonJoint, const Vector3 &);
    void SetCamJointDisplacement(SkeletonJoint, const Vector3 &);
    bool Tracked() const { return mTracked; }
    void SetTracked(bool b) { mTracked = b; }

private:
    Vector3 mCamJointPositions[kNumJoints]; // 0x4
    Vector3 mCamJointDisplacements[kNumJoints]; // 0x144
    float mCamBoneLengths[kNumBones]; // 0x284
    int mElapsedMs; // 0x2d0
    bool mTracked; // 0x2d4
};
