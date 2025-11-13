#pragma once
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

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

    ObjPtrVec<Hmx::Object> unk5c;
    DataNode unk70;
    DataNode unk78;

protected:
    FlowMultiSetProperty();
};
