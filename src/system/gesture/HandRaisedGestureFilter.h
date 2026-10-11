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
    void Update(const Skeleton &, int);
    void Update(int, int);
    void Clear();
    bool IsStandingStill() { return mStandingStillFilter.Satisfied(); }
    bool HandRaised() const { return mHandRaised; }
    bool HandPotentiallyRaised() {} // ???
    int RaisedMs() const { return mHandRaisedTimer; }

    NEW_OBJ(HandRaisedGestureFilter)

protected:
    HandRaisedGestureFilter();

private:
    bool mHandRaised; // 0x2c
    int mHandRaisedTimer; // 0x30
    int mNeedHandRaisedTime; // 0x34
    StandingStillGestureFilter mStandingStillFilter; // 0x38
};
