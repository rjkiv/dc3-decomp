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
    void Update(const Skeleton *, const Skeleton *);

    NEW_OBJ(HighFiveGestureFilter)

protected:
    HighFiveGestureFilter();

    bool mHighFived; // 0x2c
};
