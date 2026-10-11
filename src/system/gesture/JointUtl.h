#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"

bool IsSkeletonBone(const char *meshName);
const char *JointName(SkeletonJoint joint);
const char *CharBoneName(SkeletonJoint joint);
const char *MirrorBoneName(SkeletonJoint joint);
void JointScreenPos(const TrackedJoint &joint, Vector2 &pos);
void JointScreenPos(const TrackedJoint &joint, Vector3 &pos);
