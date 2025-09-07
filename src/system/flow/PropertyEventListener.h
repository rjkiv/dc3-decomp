#pragma once
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Object.h"

class PropertyEventListener {
public:
    struct AutoPropEntry {
        AutoPropEntry(Hmx::Object *obj) : unk0(0), unk4(obj) {}

        DataArray *unk0;
        ObjPtr<Hmx::Object> unk4;
    };
    PropertyEventListener(Hmx::Object *);
    virtual ~PropertyEventListener() {}

protected:
    virtual void GenerateAutoNames(FlowNode *, bool);

    void RegisterEvents(FlowNode *);
    void UnregisterEvents(FlowNode *);

    ObjVector<AutoPropEntry> mAutoPropEntries; // 0x4
    bool unk14; // 0x14
};
