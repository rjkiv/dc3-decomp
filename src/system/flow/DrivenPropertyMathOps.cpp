#include "flow/DrivenPropertyMathOps.h"

FlowMathOp::~FlowMathOp() {}

void FlowMathOp::Save(BinStream &bs) {
    bs << 6;
    bs << unk_0x4;
    bs << unk_0x0;
    bs << unk_0x18;
    bs << rhs;
    bs << lhs;
}

FlowMathOp::FlowMathOp(Hmx::Object *obj)
    : unk_0x0(0.0f), unk_0x4(0), unk_0x18(obj, nullptr) {}
