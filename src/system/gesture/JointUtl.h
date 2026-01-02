#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "vectorintrinsics.h"

void NuiTransformSkeletonToDepthImage(XMVECTOR, long *, long *, unsigned short *);
void NuiTransformSkeletonToDepthImage(XMVECTOR, float *, float *);

bool IsSkeletonBone(const char *);
const char *JointName(SkeletonJoint);
const char *CharBoneName(SkeletonJoint);
const char *MirrorBoneName(SkeletonJoint);

void JointScreenPos(const TrackedJoint &, Vector2 &);
void JointScreenPos(const TrackedJoint &, Vector3 &);
