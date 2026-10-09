#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/DancerSkeleton.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

enum ErrorNodeJoint {
    kErrorJointHipCenter = 1,
    kErrorJointSpine = 2,
    kErrorJointShoulderCenter = 4,
    kErrorJointHead = 8,
    kErrorJointShoulderLeft = 0x10,
    kErrorJointElbowLeft = 0x20,
    kErrorJointWristLeft = 0x40,
    kErrorJointHandLeft = 0x80,
    kErrorJointShoulderRight = 0x100,
    kErrorJointElbowRight = 0x200,
    kErrorJointWristRight = 0x400,
    kErrorJointHandRight = 0x800,
    kErrorJointHipLeft = 0x1000,
    kErrorJointKneeLeft = 0x2000,
    kErrorJointAnkleLeft = 0x4000,
    kErrorJointHipRight = 0x8000,
    kErrorJointKneeRight = 0x10000,
    kErrorJointAnkleRight = 0x20000,
    kErrorJointFootLeft = 0x40000,
    kErrorJointFootRight = 0x80000,
};

enum ErrorNodeType {
    kErrorHam1Euclidean = 0x100000,
    kErrorHam1Displacement = 0x200000,
    kErrorDisplacement = 0x400000,
    kErrorPosition = 0x800000,
    kNumErrorNodeTypes = 5,
};

enum ErrorScaleType {
    kErrorScaleDist = 0,
    kErrorScaleDistSq = 1
};

enum NumErrorNodes {
    // for DC2/DC3
    kMaxNumErrorNodes = 33
};

enum FeedbackLimbs {
    kFeedbackNone = 0,
    kFeedbackLeftArm = 1,
    kFeedbackRightArm = 2,
    kFeedbackLeftLeg = 4,
    kFeedbackRightLeg = 8,
    kNumLimbFeedbacks = 4
};

struct ScaleOp {
    void Set(const DataArray *);

    ErrorScaleType mType; // 0x0
    float mPerfectDist; // 0x4
    float mRate; // 0x8
};

struct ErrorFrameInput {
    ErrorFrameInput(
        const SkeletonHistory *history,
        const DancerSkeleton &desired,
        const BaseSkeleton &actual,
        float song_speed
    );

    const DancerSkeleton &mDesired; // 0x0
    const BaseSkeleton &mActual; // 0x4
    float mDesiredBoneLengths[kNumBones]; // 0x8
    float mActualBoneLengths[kNumBones]; // 0x54
    Vector3 mDesiredDisplacements[kNumJoints]; // 0xa0
    Vector3 mActualDisplacements[kNumJoints]; // 0x1e0
    bool mHasActualDisplacements; // 0x320
    Vector3 mDesiredCamJointPos[kNumJoints]; // 0x324
    Vector3 mActualCamJointPos[kNumJoints]; // 0x464
};

// Ham1NodeWeight size: 0x14
struct Ham1NodeWeight {
    bool mHasError; // 0x0
    float mPerfectDist; // 0x4
    float mRate; // 0x8
    float mAnglePerfectDist; // 0xC
    float mAngleRate; // 0x10
};

// Ham2FrameWeight size: 0x24
struct Ham2FrameWeight {
    float mWeight; // 0x0
    float mMaxFeedbackOnPSNRs[kNumLimbFeedbacks]; // 0x4
    float mMinFeedbackOffPSNRs[kNumLimbFeedbacks]; // 0x14
};

struct OldNodeWeight {
    float mWeight; // 0x0
    float mPerfectDist; // 0x4
    float mRate; // 0x8
    float mAnglePerfectDist; // 0xc
    float mAngleRate; // 0x10
};

struct ErrorNodeInput {
    void Set(const Vector3 &component_scales, const Ham1NodeWeight *node_weight);

    Vector3 mComponentScales; // 0x0
    const Ham1NodeWeight *mNodeWeight; // 0x10
};

#define kMaxNumNormBones 3

class ErrorNode {
public:
    virtual ~ErrorNode() {}
    virtual bool SkipFirstFrame() const = 0;
    virtual void
    CalcError(const ErrorFrameInput &, const ErrorNodeInput &, Vector3 &) const = 0;
    virtual void
    VizError(SkeletonViz &, const ErrorFrameInput &, const ErrorNodeInput &) const = 0;

    MEM_OVERLOAD(ErrorNode, 0x7D);
    bool IsTypeJointMatch(int type_joint_flags) const;
    bool XZErrorAxis(Vector3 &x_axis, const DancerSkeleton &dancer_skel) const;
    int GetFeedbackLimbs() const { return mFeedbackLimbs; }
    ErrorNodeType Type() const { return mType; }
    Symbol Name() const { return mName; }

    static ErrorNode *Create(const DataArray *cfg);

protected:
    ErrorNode(ErrorNodeType type, const DataArray *cfg);

    void NormBoneLengths(
        const ErrorFrameInput &frame_input,
        const SkeletonBone (&norm_bones)[3],
        float &desired_bone_len,
        float &actual_bone_len
    ) const;
    void InitNormBones(const DataArray *cfg, SkeletonBone (&bone_list)[3]);

    ErrorNodeType mType; // 0x4
    Symbol mName; // 0x8
    SkeletonJoint mSkeletonJoint; // 0xc
    /** A FeedbackLimbs bitmask of which limbs to highlight to provide feedback for. */
    int mFeedbackLimbs; // 0x10
    std::pair<SkeletonJoint, SkeletonJoint> mXZErrorJoints; // 0x14
};

// Ham1EuclideanNode size: 0x3c
class Ham1EuclideanNode : public ErrorNode {
private:
    SkeletonCoordSys mCoordSys; // 0x1c
    SkeletonJoint mBaseJoint; // 0x20
    std::pair<float, float> mComponentWeightRanges[3]; // 0x24
public:
    Ham1EuclideanNode(ErrorNodeType, const DataArray *);
    virtual bool SkipFirstFrame() const { return false; }
    virtual void CalcError(
        const ErrorFrameInput &frame_input,
        const ErrorNodeInput &node_input,
        Vector3 &errors
    ) const;
    virtual void VizError(
        SkeletonViz &viz,
        const ErrorFrameInput &frame_input,
        const ErrorNodeInput &node_input
    ) const {}
};

// BaseDisplacementNode size: 0x2c
class BaseDisplacementNode : public ErrorNode {
protected:
    struct DisplacementData {
        Vector3 mDesired; // 0x0
        Vector3 mActual; // 0x10
    };

    struct Ham1DisplacementData {
        float mActualMag; // 0x0
        Vector3 mProjected; // 0x4
        bool mPositiveProjection; // 0x14
        float mAngle; // 0x18
        float mDesiredMag; // 0x1c
    };

    bool
    Displacements(const ErrorFrameInput &frame_input, DisplacementData &disp_data) const;
    bool Displacements(
        const ErrorFrameInput &, DisplacementData &, Ham1DisplacementData &
    ) const;

private:
    SkeletonJoint mBaseJoint; // 0x1c
    SkeletonBone mNormBones[kMaxNumNormBones]; // 0x20
public:
    BaseDisplacementNode(ErrorNodeType, const DataArray *);
    virtual bool SkipFirstFrame() const { return true; }
    virtual void
    CalcError(const ErrorFrameInput &, const ErrorNodeInput &, Vector3 &) const = 0;
    virtual void
    VizError(SkeletonViz &, const ErrorFrameInput &, const ErrorNodeInput &) const = 0;
};

// DisplacementNode size: 0x2c
class DisplacementNode : public BaseDisplacementNode {
public:
    DisplacementNode(ErrorNodeType e, const DataArray *a) : BaseDisplacementNode(e, a) {}
    virtual bool SkipFirstFrame() const { return true; }
    virtual void
    CalcError(const ErrorFrameInput &, const ErrorNodeInput &, Vector3 &) const;
    virtual void
    VizError(SkeletonViz &, const ErrorFrameInput &, const ErrorNodeInput &) const {}
};

// Ham1DisplacementNode size: 0x38
class Ham1DisplacementNode : public BaseDisplacementNode {
private:
    struct ErrorData {
        float mMagError; // 0x0
        float mPotentialAngleError; // 0x4
        float mAngleError; // 0x8
    };

    ScaleOp mPotentialAngleOp; // 0x2c

    void Errors(
        const ErrorFrameInput &frame_input,
        const ErrorNodeInput &node_input,
        ErrorData &err_data,
        DisplacementData &disp_data,
        Ham1DisplacementData &ham1_disp_data
    ) const;

public:
    Ham1DisplacementNode(ErrorNodeType, const DataArray *);
    virtual bool SkipFirstFrame() const { return true; }
    virtual void
    CalcError(const ErrorFrameInput &, const ErrorNodeInput &, Vector3 &) const;
    virtual void
    VizError(SkeletonViz &, const ErrorFrameInput &, const ErrorNodeInput &) const {}
};

// PositionNode size: 0x2c
class PositionNode : public ErrorNode {
private:
    SkeletonJoint mBaseJoint; // 0x1c
    SkeletonBone mNormBones[kMaxNumNormBones]; // 0x20
public:
    PositionNode(ErrorNodeType, const DataArray *);
    virtual ~PositionNode() {}
    virtual bool SkipFirstFrame() const { return false; }
    virtual void
    CalcError(const ErrorFrameInput &, const ErrorNodeInput &, Vector3 &) const;
    virtual void
    VizError(SkeletonViz &, const ErrorFrameInput &, const ErrorNodeInput &) const {}
};

void XZErrorWeight(const Vector3 &joint_vector, float &x_weight, float &z_weight);
float ScaleDistToError(const ScaleOp &op, float dist);
float ScaleFullErrorDist(const ScaleOp &op);
