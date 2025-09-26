#include "hamobj/Pose.h"
#include "utl/Std.h"

Pose::Pose(int x, ScoreMode s) : unk18(x), mScoreMode(s) {}
Pose::~Pose() { DeleteAll(mElements); }

void Pose::AddElement(PoseElement *e) { mElements.push_back(e); }

BoneAngleRangePoseElement::BoneAngleRangePoseElement(
    SkeletonBone bone, const Vector3 &v, float f1, float f2
)
    : unk4(f2), unk8(bone), unk1c(f1) {
    Normalize(v, mAngle);
}
