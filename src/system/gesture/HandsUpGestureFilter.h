#pragma once
#include "gesture/Skeleton.h"
#include "obj/Data.h"
#include "obj/Object.h"

class HandsUpGestureFilter : public Hmx::Object {
public:
    virtual ~HandsUpGestureFilter();
    OBJ_CLASSNAME(HandsUpGestureFilter);
    OBJ_SET_TYPE(HandsUpGestureFilter);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void Update(Skeleton const &, int);
    void Update(int, int);
    void Clear();
    void SetRequiredMs(int ms) { mRequiredMs = ms; }

    NEW_OBJ(HandsUpGestureFilter)

protected:
    HandsUpGestureFilter();

    bool mHandsUp; // 0x2c
    int mRaisedMs; // 0x30
    int mRequiredMs; // 0x34
};
