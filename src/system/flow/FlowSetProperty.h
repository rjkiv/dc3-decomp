#pragma once
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"

class PropertyTask : public Hmx::Object {
public:
    PropertyTask(Hmx::Object *, DataNode &, DataNode &, TaskUnits, float, EaseType, float, bool, Hmx::Object *);
    virtual ~PropertyTask();
    OBJ_CLASSNAME(PropertyTask)
    virtual void Poll(float);

protected:
    EaseFunc *mEaseFunc;
};
