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

    bool CheckSide(int i) { return i / 2; }

    NEW_OBJ(HighFiveGestureFilter)

protected:
    HighFiveGestureFilter();

private:
    bool mHighFive; // 0x2c
};
