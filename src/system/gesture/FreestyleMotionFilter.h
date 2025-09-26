#pragma once
#include "obj/Object.h"

class FreestyleMotionFilter : public Hmx::Object {
public:
    FreestyleMotionFilter();
    virtual ~FreestyleMotionFilter();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
};
