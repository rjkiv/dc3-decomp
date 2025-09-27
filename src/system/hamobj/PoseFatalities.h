#pragma once
#include "obj/Object.h"

class PoseFatalities : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~PoseFatalities();
    OBJ_CLASSNAME(PoseFatalities);
    OBJ_SET_TYPE(PoseFatalities);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void DrawDebug();
    void Enter();
};
