#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "flow/FlowMathOp.h"
#include "utl/BinStream.h"

class FlowNode;

class DrivenPropertyEntry {
public:
    DrivenPropertyEntry(Hmx::Object *);
    ~DrivenPropertyEntry();
    void Save(BinStream &);
    void Load(BinStream &, FlowNode *);

    bool Empty() { return mMathOps.empty(); }

protected:
    DataNode unk0; // 0x0
    ObjVector<FlowMathOp> mMathOps; // 0x8
};
