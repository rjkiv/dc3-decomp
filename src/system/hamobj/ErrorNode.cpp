#include "hamobj/ErrorNode.h"
#include "ErrorNode.h"
#include "hamobj/CharFeedback.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "os/Debug.h"

ErrorNode::ErrorNode(ErrorNodeType e, const DataArray *cfg)
    : mType(e), mXErrorAxis(kJointHipCenter), mZErrorAxis(kJointHipCenter) {
    mNodeName = cfg->Sym(0);
    static Symbol joint("joint");
    static Symbol feedback_limbs("feedback_limbs");
    static Symbol xz_error_axis("xz_error_axis");
    mJoint = (SkeletonJoint)cfg->FindInt(joint);
    DataArray *limbsArr = cfg->FindArray(feedback_limbs, true);
    mFeedbackLimbs = kFeedbackNone;
    for (int i = 1; i < limbsArr->Size(); i++) {
        mFeedbackLimbs |= (FeedbackLimbs)limbsArr->Int(i);
    }
    DataArray *axisArr = cfg->FindArray(xz_error_axis, false);
    if (axisArr) {
        mXErrorAxis = (SkeletonJoint)axisArr->Int(1);
        mZErrorAxis = (SkeletonJoint)axisArr->Int(2);
    } else {
        mXErrorAxis = mZErrorAxis = kNumJoints;
    }
}

void ErrorNodeInput::Set(const Vector3 &v, const Ham1NodeWeight *w) {
    mNodeComponentWeight = v;
    mNodeWeight = w;
}

void ErrorNode::NormBoneLengths(
    const ErrorFrameInput &input,
    const SkeletonBone (&bones)[kMaxNumNormBones],
    float &f3,
    float &f4
) const {
    f3 = 0;
    f4 = 0;
    for (int i = 0; i < 3; i++) {
        SkeletonBone curBone = bones[i];
        if (curBone == kNumBones)
            return;
        f3 += input.mBoneLengths[curBone];
        f4 += input.mBaseBoneLengths[curBone];
    }
}

namespace {
    int gSkeletonJointToErrorJoint[kNumJoints] = {
        1,     2,     4,      8,      0x10,   0x20,   0x40,    0x80,    0x100,   0x200,
        0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000
    };
}

bool ErrorNode::IsTypeJointMatch(int joint) const {
    return mType & joint && gSkeletonJointToErrorJoint[mJoint] & joint;
}

ErrorFrameInput::ErrorFrameInput(
    const SkeletonHistory *history,
    const DancerSkeleton &dancerSkeleton,
    const BaseSkeleton &baseSkeleton,
    float f1
)
    : mSkeleton(dancerSkeleton), mBaseSkeleton(baseSkeleton) {
    dancerSkeleton.CamBoneLengths(mBoneLengths);
    baseSkeleton.CamBoneLengths(mBaseBoneLengths);
    dancerSkeleton.CamJointPositions(mJointPositions);
    baseSkeleton.CamJointPositions(mBaseJointPositions);
    dancerSkeleton.CamJointDisplacements(mJointDisps);
    mDisplacements = false;
    int elapsedMs = dancerSkeleton.ElapsedMs();
    if (elapsedMs != -1) {
        int div = elapsedMs / f1;
        mDisplacements =
            baseSkeleton.Displacements(history, kCoordCamera, div, mBaseJointDisps, div);
    }
}

void ErrorNode::InitNormBones(
    const DataArray *cfg, SkeletonBone (&skelBones)[kMaxNumNormBones]
) {
    static Symbol norm_bones("norm_bones");
    DataArray *bones = cfg->FindArray(norm_bones, true);
    MILO_ASSERT(bones->Size() - 1 <= kMaxNumNormBones, 0x95);
    for (int i = 0; i < kMaxNumNormBones; i++) {
        if (i < bones->Size() - 1) {
            skelBones[i] = (SkeletonBone)bones->Int(i + 1);
        } else {
            skelBones[i] = kNumBones;
        }
    }
}

bool ErrorNode::XZErrorAxis(Vector3 &v, const DancerSkeleton &skeleton) const {
    if (mXErrorAxis == kNumJoints) {
        return false;
    } else {
        Subtract(skeleton.CamJointPos(mXErrorAxis), skeleton.CamJointPos(mZErrorAxis), v);
        return true;
    }
}

ErrorNode *ErrorNode::Create(const DataArray *cfg) {
    ErrorNodeType type = (ErrorNodeType)cfg->FindInt("type");
    if (type == kErrorHam1Euclidean) {
        return new Ham1EuclideanNode(type, cfg);
    } else if (type == kErrorHam1Displacement) {
        return new Ham1DisplacementNode(type, cfg);
    } else if (type == kErrorDisplacement) {
        return new DisplacementNode(type, cfg);
    } else if (type == kErrorPosition) {
        return new PositionNode(type, cfg);
    } else {
        MILO_FAIL("Could not create node of type %i", type);
        return nullptr;
    }
}

Ham1EuclideanNode::Ham1EuclideanNode(ErrorNodeType e, const DataArray *cfg)
    : ErrorNode(e, cfg) {
    for (int i = 0; i < 3; i++) {
        mComponentWeightRanges[i][0] = 0;
        mComponentWeightRanges[i][1] = 0;
    }
    static Symbol coord_sys("coord_sys");
    mCoordSys = (SkeletonCoordSys)cfg->FindInt(coord_sys);
    static Symbol base_joint("base_joint");
    mBaseJoint = (SkeletonJoint)cfg->FindInt(base_joint);
    static Symbol component_weight_ranges("component_weight_ranges");
    DataArray *weightArr = cfg->FindArray(component_weight_ranges);
    for (int i = 1; i < 4; i++) {
        DataArray *arr = weightArr->Array(i);
        mComponentWeightRanges[i - 1][0] = arr->Float(0);
        mComponentWeightRanges[i - 1][1] = arr->Float(1);
    }
}

BaseDisplacementNode::BaseDisplacementNode(ErrorNodeType e, const DataArray *cfg)
    : ErrorNode(e, cfg) {
    static Symbol base_joint("base_joint");
    DataArray *jointArr = cfg->FindArray(base_joint, false);
    if (jointArr) {
        mBaseJoint = (SkeletonJoint)jointArr->Int(1);
    } else {
        mBaseJoint = mJoint;
    }
    InitNormBones(cfg, mNormBones);
}

Ham1DisplacementNode::Ham1DisplacementNode(ErrorNodeType e, const DataArray *cfg)
    : BaseDisplacementNode(e, cfg) {
    static Symbol potential_angle_op("potential_angle_op");
    mPotentialAngleOp.Set(cfg->FindArray(potential_angle_op));
}

PositionNode::PositionNode(ErrorNodeType e, const DataArray *cfg) : ErrorNode(e, cfg) {
    static Symbol base_joint("base_joint");

    mBaseJoint = (SkeletonJoint)cfg->FindInt(base_joint);
    InitNormBones(cfg, mNormBones);
}

void ScaleOp::Set(const DataArray *cfg) {
    static Symbol type("type");
    static Symbol rate("rate");
    static Symbol perfect_dist("perfect_dist");
    mType = (ErrorScaleType)cfg->FindInt(type);
    cfg->FindData(perfect_dist, mPerfectDist);
    cfg->FindData(rate, mRate);
}
