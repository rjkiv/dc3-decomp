#pragma once
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"
#include <vector>

class FlowMultiSetProperty : public FlowNode {
public:
    // Hmx::Object
    virtual ~FlowMultiSetProperty();
    OBJ_CLASSNAME(FlowMultiSetProperty)
    OBJ_SET_TYPE(FlowMultiSetProperty)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // FlowNode
    virtual bool Activate();

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(FlowMultiSetProperty)

    ObjPtrVec<Hmx::Object> mTargetObjects; // 0x5c
    DataNode mProperty; // 0x70
    DataNode mValue; // 0x78

protected:
    FlowMultiSetProperty();
};
