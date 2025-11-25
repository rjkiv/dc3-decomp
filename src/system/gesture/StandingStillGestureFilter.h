#pragma once
#include "gesture/Skeleton.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "types.h"

class StandingStillGestureFilter : public Hmx::Object {
public:
    virtual ~StandingStillGestureFilter();
    OBJ_CLASSNAME(StandingStillGestureFilter)
    OBJ_SET_TYPE(StandingStillGestureFilter)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    StandingStillGestureFilter();
    void SetForwardFacingCutoff(float);
    void RestoreDefaultForwardFacingCutoff();
    void Update(Skeleton const &, int);
    void Update(int, int);
    void Clear();

    bool mHasRequiredMs; // 0x2c
    int mRaisedMs; // 0x30
    int unk34;
    u32 unk38;
    u32 unk3c;
    u32 unk40;
    u32 unk44;
    float mForwardFacingCutoff; // 0x48
    bool unk4c;
};
