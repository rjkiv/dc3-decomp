#include "hamobj/Pose.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "os/Debug.h"
#include "utl/Std.h"

Pose::Pose(int x, ScoreMode s) : unk18(x), mScoreMode(s) {}
Pose::~Pose() { DeleteAll(mElements); }

void Pose::AddElement(PoseElement *e) { mElements.push_back(e); }

float Pose::CurrentScore() const {
    float f6 = 0;
    float f5 = 1;
    FOREACH (it, unk10) {
        f6 += *it;
        MinEq(f5, *it);
    }
    switch (mScoreMode) {
    case 0:
        return f6 / (float)unk18;
    case 1:
        return unk10.size() < unk18 ? 0 : f5;
    default:
        MILO_FAIL("Bad Pose ScoreMode!");
        return 0;
    }
}

void Pose::Update(const Skeleton &skeleton) {
    MILO_ASSERT(!mElements.empty(), 0x8A);
    float f9 = 0;
    float totalWeight = 0;
    FOREACH (it, mElements) {
        PoseElement *cur = *it;
        f9 += cur->Score(skeleton) * cur->Weight();
        totalWeight += cur->Weight();
    }
    MILO_ASSERT(totalWeight != 0.0f, 149);
    unk10.push_back(f9 / totalWeight);
    if (unk10.size() > unk18) {
        unk10.pop_front();
    }
}

BoneAngleRangePoseElement::BoneAngleRangePoseElement(
    SkeletonBone bone, const Vector3 &v, float f1, float f2
)
    : PoseElement(f2), unk8(bone), unk1c(f1) {
    Normalize(v, mAngle);
}

float BoneAngleRangePoseElement::Score(const Skeleton &skeleton) const {
    Vector3 v30;
    skeleton.BoneVec(unk8, kCoordCamera, v30);
    Normalize(v30, v30);
    MILO_ASSERT((1.0f)-(0.001f) <= (Length(mAngle)) && (Length(mAngle)) <= (1.0f)+(0.001f), 0x21);
    float dot = Dot(v30, mAngle);
    float a = acosf(dot);
    if (a <= unk1c) {
        return 1;
    } else {
        return 0;
    }
}

float CamDistancePoseElement::Score(const Skeleton &skeleton) const {
    Vector3 v20;
    skeleton.JointPos(kCoordCamera, kJointSpine, v20);
    if (v20.z > unk8) {
        return 1;
    } else {
        return 0;
    }
}

JointDistPoseElement::JointDistPoseElement(
    SkeletonJoint j1, SkeletonJoint j2, float minDist, float maxDist
)
    : PoseElement(1), unk8(j1), unkc(j2), unk10(minDist), unk14(maxDist),
      unk18(kCoordCamera) {
    MILO_ASSERT(minDist <= maxDist, 0x2F);
}

float JointDistPoseElement::Score(const Skeleton &skeleton) const {
    Vector3 v40;
    skeleton.JointPos(unk18, unk8, v40);
    Vector3 v30;
    skeleton.JointPos(unk18, unkc, v30);
    float dist = Distance(v40, v30);
    if (dist >= unk10 && dist <= unk14) {
        return 1;
    } else {
        return 0;
    }
}
