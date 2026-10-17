#pragma once
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Object.h"

class PropertyEventListener {
public:
    struct AutoPropEntry {
        AutoPropEntry(Hmx::Object *obj) : mPropPath(0), mProvider(obj) {}

        DataArray *mPropPath; // 0x0
        ObjPtr<Hmx::Object> mProvider; // 0x4
    };
    PropertyEventListener(Hmx::Object *);
    virtual ~PropertyEventListener() {}

protected:
    virtual void GenerateAutoNames(FlowNode *, bool);

    void RegisterEvents(FlowNode *);
    void UnregisterEvents(FlowNode *);

    ObjVector<AutoPropEntry> mAutoPropEntries; // 0x4
    bool mRegistered; // 0x14
};
