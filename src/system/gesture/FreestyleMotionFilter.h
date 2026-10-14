#pragma once
#include "gesture/Skeleton.h"
#include "obj/Object.h"

class FreestyleMotionFilter : public Hmx::Object {
public:
    FreestyleMotionFilter();
    // Hmx::Object
    virtual ~FreestyleMotionFilter();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void Clear();
    void Activate();
    void Deactivate();
    bool IsActive() const;
    bool Detected();
    void UpdateFilters(const SkeletonUpdateData &data);

private:
    float mVelocityThreshold; // 0x2c
    float mFrameMoveMs; // 0x30
    float mMovement; // 0x34
    bool mActive; // 0x38
};
