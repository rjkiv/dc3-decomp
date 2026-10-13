#pragma once
#include "ArcDetector.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "gesture/StandingStillGestureFilter.h"
#include "rndobj/Overlay.h"
#include "gesture/SkeletonViz.h"

class DirectionGestureFilter : public RndOverlay::Callback {
public:
    virtual ~DirectionGestureFilter() {}
    virtual void Clear() = 0;
    virtual JointConfidence Confidence() const = 0;
    virtual void Update(const Skeleton &skeleton, int elapsed_ms) = 0;
    virtual void Draw(const Skeleton &skeleton, SkeletonViz &viz) = 0;
    virtual bool HasDirection() const = 0;
    virtual float GetPercentPulled() const = 0;
    virtual bool IsHandValid(const Skeleton &skeleton) const = 0;
    virtual bool IsValidScrollPos(const Skeleton &skeleton) const = 0;
    virtual void ClearSwipe() = 0;
    virtual bool IsLockedIn() const = 0;
    virtual void SetEngaged(bool engaged) = 0;
    virtual void ResetHoverTimer() = 0;
    virtual void SetAllowAboveShoulder(bool allow) = 0;
    virtual void SetHighButtonMode(bool enabled) = 0;

protected:
    static float sLastSwipeTime[6];
};

class DirectionGestureFilterSingleUser : public DirectionGestureFilter {
public:
    DirectionGestureFilterSingleUser(
        SkeletonSide side,
        SkeletonSide direction,
        float swipeDistance,
        float startOutsideShoulder
    );
    virtual ~DirectionGestureFilterSingleUser();
    virtual void Clear();
    virtual JointConfidence Confidence() const { return mConfidence; }
    virtual void Update(const Skeleton &, int);
    virtual void Draw(const Skeleton &, SkeletonViz &);
    virtual bool HasDirection() const { return mHasDirection; }
    virtual float GetPercentPulled() const { return mDisplayedPercentPulled; }
    virtual bool IsHandValid(const Skeleton &skeleton) const;
    virtual bool IsValidScrollPos(const Skeleton &skeleton) const;
    virtual void ClearSwipe();
    virtual bool IsLockedIn() const;
    virtual void SetEngaged(bool engaged) { mEngaged = engaged; }
    virtual void ResetHoverTimer();
    virtual void SetAllowAboveShoulder(bool allow) { mAllowAboveShoulder = allow; }
    virtual void SetHighButtonMode(bool enabled) { mHighButtonMode = enabled; }

private:
    virtual float UpdateOverlay(RndOverlay *o, float topY);

    bool
    HandAtSide(const Skeleton &skeleton, float distance, float shape, float xshift) const;
    bool IsValidSwipePosition(const Skeleton &skeleton) const;

    SkeletonSide mSide; // 0x4
    JointConfidence mConfidence; // 0x8
    SkeletonSide mDirection; // 0xc
    bool mHasDirection; // 0x10
    float mPercentPulled; // 0x14
    float mDisplayedPercentPulled; // 0x18
    bool mEngaged; // 0x1c
    bool mAllowAboveShoulder; // 0x1d
    bool mHighButtonMode; // 0x1e
    float mTimeBetweenSwipes; // 0x20
    ArcDetector mHandArcDetector; // 0x24
};

class DirectionGestureFilterDoubleUser : public DirectionGestureFilter {
public:
    DirectionGestureFilterDoubleUser(
        SkeletonSide side,
        SkeletonSide direction,
        float swipeDistance,
        float startOutsideShoulder
    );
    virtual ~DirectionGestureFilterDoubleUser();
    virtual void Clear();
    virtual JointConfidence Confidence() const;
    virtual void Update(const Skeleton &, int);
    virtual void Draw(const Skeleton &, SkeletonViz &);
    virtual bool HasDirection() const;
    virtual float GetPercentPulled() const;
    virtual bool IsHandValid(const Skeleton &) const;
    virtual bool IsValidScrollPos(const Skeleton &) const;
    virtual void ClearSwipe();
    virtual bool IsLockedIn() const;
    virtual void SetEngaged(bool engaged);
    virtual void ResetHoverTimer();
    virtual void SetAllowAboveShoulder(bool allow);
    virtual void SetHighButtonMode(bool enabled);

private:
    void GetValidSkeletons(int &skel1, int &skel2) const;

    DirectionGestureFilterSingleUser *mFilter1; // 0x4
    DirectionGestureFilterSingleUser *mFilter2; // 0x8
    StandingStillGestureFilter *mStandingStillFilters[2]; // 0xc
};
