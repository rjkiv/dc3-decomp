#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "flow/DrivenPropertyMathOps.h"
#include "utl/BinStream.h"

class FlowNode;

class DrivenPropertyEntry {
public:
    DrivenPropertyEntry(Hmx::Object *);
    ~DrivenPropertyEntry();
    void Save(BinStream &);
    void Load(BinStream &, FlowNode *);

    DataNode mDrivenProperty; // 0x0
    ObjVector<FlowMathOp> mMathOps; // 0x8
};
