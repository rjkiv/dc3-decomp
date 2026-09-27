#include "hamobj/ErrorNode.h"
#include "ErrorNode.h"
#include "hamobj/CharFeedback.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/DancerSkeleton.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "os/Debug.h"

namespace {
    int gSkeletonJointToErrorJoint[kNumJoints] = {
        kErrorJointHipCenter,  kErrorJointSpine,        kErrorJointShoulderCenter,
        kErrorJointHead,       kErrorJointShoulderLeft, kErrorJointElbowLeft,
        kErrorJointWristLeft,  kErrorJointHandLeft,     kErrorJointShoulderRight,
        kErrorJointElbowRight, kErrorJointWristRight,   kErrorJointHandRight,
        kErrorJointHipLeft,    kErrorJointKneeLeft,     kErrorJointAnkleLeft,
        kErrorJointHipRight,   kErrorJointKneeRight,    kErrorJointAnkleRight,
        kErrorJointFootLeft,   kErrorJointFootRight
    };
}

void DistanceToErrors(
    const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, Vector3 &v4
) {
    Subtract(v1, v2, v4);
    Scale(v4, v3, v4);
    for (int i = 0; i < 3; i++) {
        v4[i] = Clamp(0.0f, 1.0f, fabsf(v4[i]));
    }
}

float ScaleDistToError(const ScaleOp &op, float f2) {
    if (op.mPerfectDist != -1) {
        if (f2 < 0) {
            MILO_NOTIFY(
                "%f distance is less than zero (%f, %f)", f2, op.mPerfectDist, op.mRate
            );
            return 1;
        } else if (f2 > op.mPerfectDist) {
            float f1 = f2 - op.mPerfectDist;
            if (op.mType == kErrorScaleDistSq) {
                f1 *= f1;
            }
            f2 = op.mRate * f1;
        } else {
            return 0;
        }
    }
    return f2;
}

float ScaleFullErrorDist(const ScaleOp &op) {
    if (op.mPerfectDist == -1) {
        return 1;
    } else if (op.mType == kErrorScaleDist) {
        float f1 = 1 / op.mRate;
        return f1 + op.mPerfectDist;
    } else {
        float f1 = sqrtf(1 / op.mRate);
        return f1 + op.mPerfectDist;
    }
}

void XZErrorWeight(const Vector3 &v3, float &f2, float &f3) {
    Vector3 v = v3;
    v.y = 0;
    Normalize(v, v);
    static Vector3 sVec(0, 0, 1);
    float dot = acosf(fabsf(Dot(sVec, v)));
    f2 = dot * 0.63661975f;
    f3 = 1 - (dot * 0.63661975f);
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
        int div;
        mDisplacements = baseSkeleton.Displacements(
            history, kCoordCamera, elapsedMs / f1, mBaseJointDisps, div
        );
    }
}

void ErrorNodeInput::Set(const Vector3 &v, const Ham1NodeWeight *w) {
    mNodeComponentWeight = v;
    mNodeWeight = w;
}

#pragma region ErrorNode

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

void ErrorNode::NormBoneLengths(
    const ErrorFrameInput &input,
    const SkeletonBone (&bones)[kMaxNumNormBones],
    float &totalBoneLengths,
    float &totalBaseBoneLengths
) const {
    totalBoneLengths = 0;
    totalBaseBoneLengths = 0;
    for (int i = 0; i < 3; i++) {
        SkeletonBone curBone = bones[i];
        if (curBone == kNumBones)
            return;
        totalBoneLengths += input.mBoneLengths[curBone];
        totalBaseBoneLengths += input.mBaseBoneLengths[curBone];
    }
}

bool ErrorNode::IsTypeJointMatch(int joint) const {
    return mType & joint && gSkeletonJointToErrorJoint[mJoint] & joint;
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

#pragma endregion
#pragma region Ham1EuclideanNode

Ham1EuclideanNode::Ham1EuclideanNode(ErrorNodeType e, const DataArray *cfg)
    : ErrorNode(e, cfg) {
    for (int i = 0; i < 3; i++) {
        // 0x24, 0x28 i = 0
        // 0x2c, 0x30 i = 1
        // 0x34, 0x38 i = 2
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
        float *curRange = mComponentWeightRanges[i - 1];
        curRange[0] = arr->Float(0);
        curRange[1] = arr->Float(1);
    }
}

void Ham1EuclideanNode::CalcError(
    const ErrorFrameInput &frame_input, const ErrorNodeInput &node_input, Vector3 &vout
) const {
    MILO_ASSERT(node_input.mNodeWeight, 0x10E);
    Vector3 dancerVec;
    frame_input.mSkeleton.NormPos(mCoordSys, mJoint, dancerVec);
    Vector3 baseVec;
    frame_input.mBaseSkeleton.NormPos(mCoordSys, mJoint, baseVec);
    Vector3 diff;
    Subtract(dancerVec, baseVec, diff);
    Vector3 vToProcess;
    for (int i = 0; i < 3; i++) {
        vToProcess[i] = Clamp(
            mComponentWeightRanges[i][0],
            mComponentWeightRanges[i][1],
            node_input.mNodeComponentWeight[i]
        );
    }
    ScaleOp op;
    op.mPerfectDist = node_input.mNodeWeight->unk4;
    op.mType = kErrorScaleDistSq;
    op.mRate = node_input.mNodeWeight->unk8;
    Vector3 tmp;
    Scale(vToProcess, diff, tmp);
    vout.x = ScaleDistToError(op, Length(tmp));
}

#pragma endregion
#pragma region BaseDisplacementNode

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

bool BaseDisplacementNode::Displacements(
    const ErrorFrameInput &frame_input, BaseDisplacementNode::DisplacementData &dispData
) const {
    if (frame_input.mDisplacements) {
        dispData.mJointDisplacement = frame_input.mJointDisps[mJoint];
        dispData.mBaseJointDisplacement = frame_input.mBaseJointDisps[mJoint];
        if (mBaseJoint != mJoint) {
            Subtract(
                dispData.mJointDisplacement,
                frame_input.mJointDisps[mBaseJoint],
                dispData.mJointDisplacement
            );
            Subtract(
                dispData.mBaseJointDisplacement,
                frame_input.mBaseJointDisps[mBaseJoint],
                dispData.mBaseJointDisplacement
            );
        }
        float boneLen, baseBoneLen;
        NormBoneLengths(frame_input, mNormBones, boneLen, baseBoneLen);
        if (baseBoneLen > 0) {
            Scale(
                dispData.mBaseJointDisplacement,
                boneLen / baseBoneLen,
                dispData.mBaseJointDisplacement
            );
            return true;
        }
    }
    dispData.mBaseJointDisplacement.Zero();
    dispData.mJointDisplacement.Zero();
    return false;
}

bool BaseDisplacementNode::Displacements(
    const ErrorFrameInput &frame_input,
    BaseDisplacementNode::DisplacementData &dispData,
    BaseDisplacementNode::Ham1DisplacementData &ham1Data
) const {
    ham1Data.unk4.Zero();
    ham1Data.unk14 = false;
    ham1Data.unk18 = 0;
    ham1Data.unk0 = 0;
    ham1Data.unk1c = 0;
    if (Displacements(frame_input, dispData)) {
        float jointDispLen = Length(dispData.mJointDisplacement);
        ham1Data.unk1c = jointDispLen;

        Vector3 invJointDisp;
        if (jointDispLen > 0) {
            Scale(dispData.mJointDisplacement, 1 / jointDispLen, invJointDisp);
        } else {
            invJointDisp.Set(0, 0, 0);
        }

        float jointDot = Dot(invJointDisp, dispData.mBaseJointDisplacement);
        Scale(invJointDisp, jointDot, ham1Data.unk4);
        ham1Data.unk14 = jointDot > 0;

        float baseJointDispLen = Length(dispData.mBaseJointDisplacement);
        ham1Data.unk0 = baseJointDispLen;

        Vector3 invBaseJointDisp;
        if (baseJointDispLen > 0) {
            Scale(dispData.mBaseJointDisplacement, 1 / baseJointDispLen, invBaseJointDisp);
        } else {
            invBaseJointDisp.Set(0, 0, 0);
        }

        ham1Data.unk18 =
            fabsf(acosf(Clamp(-1.0f, 1.0f, Dot(invBaseJointDisp, invJointDisp))));

        return true;
    } else {
        return false;
    }
}

void DisplacementNode::CalcError(
    const ErrorFrameInput &frame_input, const ErrorNodeInput &node_input, Vector3 &vout
) const {
    MILO_ASSERT(node_input.mNodeWeight == NULL, 0x1F8);
    DisplacementData dispData;
    if (Displacements(frame_input, dispData)) {
        DistanceToErrors(
            dispData.mJointDisplacement,
            dispData.mBaseJointDisplacement,
            node_input.mNodeComponentWeight,
            vout
        );
    } else {
        vout.Set(1, 1, 1);
    }
}

Ham1DisplacementNode::Ham1DisplacementNode(ErrorNodeType e, const DataArray *cfg)
    : BaseDisplacementNode(e, cfg) {
    static Symbol potential_angle_op("potential_angle_op");
    mPotentialAngleOp.Set(cfg->FindArray(potential_angle_op));
}

void Ham1DisplacementNode::CalcError(
    const ErrorFrameInput &frame_input, const ErrorNodeInput &node_input, Vector3 &vout
) const {
    MILO_ASSERT(node_input.mNodeWeight, 0x19D);
    ErrorData errData;
    DisplacementData dispData;
    Ham1DisplacementData ham1DispData;
    Errors(frame_input, node_input, errData, dispData, ham1DispData);
    vout.x = errData.unk4 * errData.unk8 + errData.unk0;
}

void Ham1DisplacementNode::Errors(
    const ErrorFrameInput &frame_input,
    const ErrorNodeInput &node_input,
    ErrorData &errorData,
    DisplacementData &dispData,
    Ham1DisplacementData &ham1DispData
) const {
    if (!Displacements(frame_input, dispData, ham1DispData)) {
        errorData.unk8 = 1;
        errorData.unk0 = 1;
        errorData.unk4 = 1;
    } else {
        float u18 = ham1DispData.unk18;
        ScaleOp op;
        op.mType = kErrorScaleDistSq;
        op.mPerfectDist = node_input.mNodeWeight->unkc;
        op.mRate = node_input.mNodeWeight->unk10;
        float len = Length(ham1DispData.unk4);
        errorData.unk4 = ScaleDistToError(mPotentialAngleOp, ham1DispData.unk0);
        MinEq(errorData.unk4, 1.0f);
        errorData.unk8 = ScaleDistToError(op, u18);
        float f2 = 1;
        if (ham1DispData.unk1c > 0) {
            f2 = len / ham1DispData.unk1c;
        }
        float f6;
        if (ham1DispData.unk14) {
            f6 = fabsf(1 - f2);
        } else {
            f6 = f2 + 1;
        }
        op.mType = kErrorScaleDistSq;
        op.mPerfectDist = node_input.mNodeWeight->unk4;
        op.mRate = node_input.mNodeWeight->unk8;
        errorData.unk0 = ScaleDistToError(op, f6);
    }
}

PositionNode::PositionNode(ErrorNodeType e, const DataArray *cfg) : ErrorNode(e, cfg) {
    static Symbol base_joint("base_joint");

    mBaseJoint = (SkeletonJoint)cfg->FindInt(base_joint);
    InitNormBones(cfg, mNormBones);
}

void PositionNode::CalcError(
    const ErrorFrameInput &frame_input, const ErrorNodeInput &node_input, Vector3 &vout
) const {
    MILO_ASSERT(node_input.mNodeWeight == NULL, 0x21C);
    Vector3 jointDiff;
    Subtract(
        frame_input.mJointPositions[mJoint],
        frame_input.mJointPositions[mBaseJoint],
        jointDiff
    );
    Vector3 baseJointDiff;
    Subtract(
        frame_input.mBaseJointPositions[mJoint],
        frame_input.mBaseJointPositions[mBaseJoint],
        baseJointDiff
    );
    float desired_bone_len;
    float base_bone_len;
    NormBoneLengths(frame_input, mNormBones, desired_bone_len, base_bone_len);
    MILO_ASSERT(desired_bone_len > 0, 0x22C);
    if (base_bone_len <= 0) {
        vout.Set(1, 1, 1);
    } else {
        Vector3 scaledBaseDiff;
        float scalar = desired_bone_len / base_bone_len;
        Scale(baseJointDiff, scalar, scaledBaseDiff);
        DistanceToErrors(jointDiff, scaledBaseDiff, node_input.mNodeComponentWeight, vout);
    }
}

void ScaleOp::Set(const DataArray *cfg) {
    static Symbol type("type");
    static Symbol rate("rate");
    static Symbol perfect_dist("perfect_dist");
    mType = (ErrorScaleType)cfg->FindInt(type);
    cfg->FindData(perfect_dist, mPerfectDist);
    cfg->FindData(rate, mRate);
}
