#pragma once
#include "char/Character.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/CameraInput.h"
#include "gesture/Skeleton.h"
#include "rndobj/Trans.h"

class CharCameraInput : public CameraInput {
public:
    CharCameraInput(Character *);
    // CameraInput
    virtual float DrawScale() const { return kDrawScale; }
    virtual bool NatalToWorld(Transform &) const;

    void ResetSkeletonCharOrigin();
    void SetFlip(bool flip) { mFlip = flip; }

    static float const kDrawScale;

protected:
    virtual const SkeletonFrame *PollNewFrame();

    Character *mChar; // 0x11d4
    SkeletonFrame mFrame; // 0x11d8
    RndTransformable *mBoneNames[kNumJoints]; // 0x23a0
    Transform mNatalXfm; // 0x23f0
    bool mFlip; // 0x2430
};
