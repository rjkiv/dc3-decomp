#pragma once
#include "gesture/Skeleton.h"
#include "obj/Object.h"

class HighFiveGestureFilter : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~HighFiveGestureFilter();
    OBJ_CLASSNAME(HighFiveGestureFilter)
    OBJ_SET_TYPE(HighFiveGestureFilter)

    bool CheckHighFive();
    void Update(Skeleton const *, Skeleton const *);

    NEW_OBJ(HighFiveGestureFilter)

    bool unk2c; // 0x2c

protected:
    HighFiveGestureFilter();
};
