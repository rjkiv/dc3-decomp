#include "FlowSetProperty.h"
#include "math/Easing.h"
#include "math/Trig.h"
#include <cmath>
#include "math/Rot.h"

PropertyTask::
    PropertyTask(Hmx::Object *, DataNode &, DataNode &, TaskUnits, float, EaseType t, float, bool, Hmx::Object *) {
    mEaseFunc = GetEaseFunction(t);
}
