#pragma once
#include "gesture/Skeleton.h"
#include "obj/Data.h"
#include "obj/Object.h"

class StandingStillGestureFilter : public Hmx::Object {
public:
    StandingStillGestureFilter();
    virtual ~StandingStillGestureFilter();
    OBJ_CLASSNAME(StandingStillGestureFilter)
    OBJ_SET_TYPE(StandingStillGestureFilter)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void SetForwardFacingCutoff(float);
    void RestoreDefaultForwardFacingCutoff();
    void Update(const Skeleton &, int);
    void Update(int, int);
    void Clear();
    void SetRequiredMs(int ms) { mRequiredMs = ms; }
    void SetRaisedMs(int ms) { mRaisedMs = ms; }
    void SetUnk4c(bool b) { unk4c = b; }

    bool Satisfied() const { return mSatisfied; }
    bool PotentiallySatisfied() {} // ???
    int RaisedMs() const { return mRaisedMs; }
    int RequiredMs() const { return mRequiredMs; }

private:
    bool mSatisfied; // 0x2c
    int mRaisedMs; // 0x30
    int mRequiredMs; // 0x34
    Vector3 mCenter;
    float mForwardFacingCutoff; // 0x48
    bool unk4c; // not in PDB?
};
