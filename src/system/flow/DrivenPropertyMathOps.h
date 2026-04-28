#pragma once
#include "flow/FlowPtr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "types.h"
#include "utl/BinStream.h"

class FlowMathOp {
    float unk_0x0;
    u32 unk_0x4;
    DataNode lhs; // 0x8
    DataNode rhs; // 0x10
    FlowPtr<Hmx::Object> unk_0x18;

public:
    FlowMathOp(Hmx::Object *);
    FlowMathOp(const FlowMathOp &other)
        : unk_0x0(other.unk_0x0), unk_0x4(other.unk_0x4), lhs(other.lhs), rhs(other.rhs),
          unk_0x18(other.unk_0x18) {}
    ~FlowMathOp();
    void Save(BinStream &);
    void Load(BinStream &, ObjectDir *);

    float Apply(float);

    float GetUnk0() const { return unk_0x0; }
    const DataNode &LHS() const { return lhs; }
    const DataNode &RHS() const { return rhs; }
    FlowPtr<Hmx::Object> &GetUnk18() { return unk_0x18; }
};
