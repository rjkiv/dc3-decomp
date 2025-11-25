#pragma once
#include "Skeleton.h"
#include "StandingStillGestureFilter.h"
#include "obj/Data.h"
#include "obj/Object.h"

class HandRaisedGestureFilter : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~HandRaisedGestureFilter();
    OBJ_CLASSNAME(HandRaisedGestureFilter)
    OBJ_SET_TYPE(HandRaisedGestureFilter)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void SetRequiredMs(int);
    void SetForwardFacingCutoff(float);
    void RestoreDefaultForwardFacingCutoff();
    void Update(Skeleton const &, int);
    void Update(int, int);
    void Clear();

    bool unk2c;
    int unk30;
    int mRequiredMs; // 0x34
    StandingStillGestureFilter unk38;

protected:
    HandRaisedGestureFilter();
};
