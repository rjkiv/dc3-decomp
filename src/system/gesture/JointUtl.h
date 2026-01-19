#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"

bool IsSkeletonBone(const char *);
const char *JointName(SkeletonJoint);
const char *CharBoneName(SkeletonJoint);
const char *MirrorBoneName(SkeletonJoint);
void JointScreenPos(const TrackedJoint &, Vector2 &);
void JointScreenPos(const TrackedJoint &, Vector3 &);
