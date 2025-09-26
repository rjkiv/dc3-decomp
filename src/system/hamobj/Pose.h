#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include <vector>
#include <list>

class PoseElement {
public:
    virtual ~PoseElement() {}
    virtual float Score(const Skeleton &) const = 0;
};

class CamDistancePoseElement : public PoseElement {
public:
    CamDistancePoseElement(float f1, float f2) : unk4(f1), unk8(f2) {}
    virtual float Score(const Skeleton &) const;

protected:
    float unk4;
    float unk8;
};

class JointDistPoseElement : public PoseElement {
public:
    JointDistPoseElement(SkeletonJoint, SkeletonJoint, float, float);
    virtual float Score(const Skeleton &) const;

protected:
    float unk4;
    SkeletonJoint unk8;
    SkeletonJoint unkc;
    float unk10;
    float unk14;
    int unk18;
};

class BoneAngleRangePoseElement : public PoseElement {
public:
    BoneAngleRangePoseElement(SkeletonBone, const Vector3 &, float, float);
    virtual float Score(const Skeleton &) const;

protected:
    float unk4; // 0x4
    SkeletonBone unk8; // 0x8
    Vector3 mAngle; // 0xc
    float unk1c; // 0x1c
};

class Pose {
public:
    enum ScoreMode {
    };
    Pose(int x, ScoreMode s);
    virtual ~Pose();

    void AddElement(PoseElement *);
    float CurrentScore() const;
    void Update(const Skeleton &);

protected:
    std::vector<PoseElement *> mElements; // 0x4
    std::list<float> unk10; // 0x10
    int unk18; // 0x18
    ScoreMode mScoreMode; // 0x1c
};
