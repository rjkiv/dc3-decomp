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

    bool Empty() { return mMathOps.empty(); }
    const DataNode &Node() const { return unk0; }
    ObjVector<FlowMathOp> &MathOps() { return mMathOps; }
    const ObjVector<FlowMathOp> &MathOps() const { return mMathOps; }

protected:
    DataNode unk0; // 0x0
    ObjVector<FlowMathOp> mMathOps; // 0x8
};
