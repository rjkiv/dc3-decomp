#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/DancerSkeleton.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

enum ErrorNodeType {
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
    kErrorHam1Euclidean = 0x100000,
    kErrorHam1Displacement = 0x200000,
    kErrorDisplacement = 0x400000,
    kErrorPosition = 0x800000
};

enum ErrorScaleType {
    kErrorScaleDist = 0,
    kErrorScaleDistSq = 1
};

enum NumErrorNodes {
    // for DC2/DC3
    kMaxNumErrorNodes = 33
};

struct ScaleOp {
    void Set(const DataArray *);

    ErrorScaleType mType; // 0x0
    float mPerfectDist; // 0x4
    float mRate; // 0x8
};

struct ErrorFrameInput {
    ErrorFrameInput(
        const SkeletonHistory *, const DancerSkeleton &, const BaseSkeleton &, float
    );

    const DancerSkeleton &mSkeleton; // 0x0
    const BaseSkeleton &mBaseSkeleton; // 0x4
    float mBoneLengths[kNumBones]; // 0x8
    float mBaseBoneLengths[kNumBones]; // 0x54
    Vector3 mJointDisps[kNumJoints]; // 0xa0
    Vector3 mBaseJointDisps[kNumJoints]; // 0x1e0
    bool mDisplacements; // 0x320
    Vector3 mJointPositions[kNumJoints]; // 0x324
    Vector3 mBaseJointPositions[kNumJoints]; // 0x464
};

// Ham1NodeWeight size: 0x14
struct Ham1NodeWeight {
    bool unk0;
    float unk4; // seen this assigned to ScaleOp's mPerfectDist
    float unk8; // seen this assigned to ScaleOp's mRate
    float unkc; // seen this assigned to ScaleOp's mPerfectDist
    float unk10; // seen this assigned to ScaleOp's mRate
};

// Ham2FrameWeight size: 0x24
struct Ham2FrameWeight {
    float unk0;
    float unk4[4];
    float unk14[4];
};

struct OldNodeWeight {
    float unk0;
    float unk4, unk8, unkc, unk10;
};

struct ErrorNodeInput {
    void Set(const Vector3 &, const Ham1NodeWeight *);

    Vector3 mNodeComponentWeight; // 0x0
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
    bool IsTypeJointMatch(int) const;
    bool XZErrorAxis(Vector3 &, const DancerSkeleton &) const;

    static ErrorNode *Create(const DataArray *);

protected:
    ErrorNode(ErrorNodeType, const DataArray *);

    void
    NormBoneLengths(const ErrorFrameInput &, const SkeletonBone (&)[3], float &, float &)
        const;
    void InitNormBones(const DataArray *, SkeletonBone (&)[3]);

    ErrorNodeType mType; // 0x4
    Symbol mNodeName; // 0x8
    SkeletonJoint mJoint; // 0xc
    int mFeedbackLimbs; // 0x10
    SkeletonJoint mXErrorAxis; // 0x14
    SkeletonJoint mZErrorAxis; // 0x18
};

// Ham1EuclideanNode size: 0x3c
class Ham1EuclideanNode : public ErrorNode {
private:
    SkeletonCoordSys mCoordSys; // 0x1c
    SkeletonJoint mBaseJoint; // 0x20
    float mComponentWeightRanges[3][2]; // 0x24
public:
    Ham1EuclideanNode(ErrorNodeType, const DataArray *);
    virtual bool SkipFirstFrame() const { return false; }
    virtual void
    CalcError(const ErrorFrameInput &, const ErrorNodeInput &, Vector3 &) const;
    virtual void
    VizError(SkeletonViz &, const ErrorFrameInput &, const ErrorNodeInput &) const {}
};

// BaseDisplacementNode size: 0x2c
class BaseDisplacementNode : public ErrorNode {
protected:
    struct DisplacementData {
        Vector3 mJointDisplacement;
        Vector3 mBaseJointDisplacement;
    };

    struct Ham1DisplacementData {
        float unk0;
        Vector3 unk4;
        bool unk14;
        float unk18;
        float unk1c;
    };

    bool Displacements(const ErrorFrameInput &, DisplacementData &) const;
    bool
    Displacements(const ErrorFrameInput &, DisplacementData &, Ham1DisplacementData &)
        const;

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
        float unk0;
        float unk4;
        float unk8;
    };

    ScaleOp mPotentialAngleOp; // 0x2c

    void
    Errors(const ErrorFrameInput &, const ErrorNodeInput &, ErrorData &, DisplacementData &, Ham1DisplacementData &)
        const;

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

void XZErrorWeight(const Vector3 &, float &, float &);
float ScaleDistToError(const ScaleOp &, float);
float ScaleFullErrorDist(const ScaleOp &);
