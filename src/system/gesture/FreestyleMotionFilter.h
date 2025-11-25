#pragma once
#include "gesture/Skeleton.h"
#include "obj/Object.h"

class FreestyleMotionFilter : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~FreestyleMotionFilter();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    FreestyleMotionFilter();
    void Clear();
    void Activate();
    void Deactivate();
    bool IsActive() const;
    bool Detected();
    void UpdateFilters(SkeletonUpdateData const &);

    float unk2c;
    float unk30;
    float unk34;
    bool mIsActive; // 0x38
};
