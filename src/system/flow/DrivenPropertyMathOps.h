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
    ~FlowMathOp();
    void Save(BinStream &);
    void Load(BinStream &, ObjectDir *);
};
