#pragma once
#include "flow/FlowPtr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "types.h"
#include "utl/BinStream.h"

class FlowMathOp {
public:
    enum MathOpTypes {
        kInitValue = -1,
        kAdd = 0,
        kSubtract = 1,
        kMultiply = 2,
        kDivide = 3,
        kVariance = 4,
        kClampMin = 5,
        kClampMax = 6,
        kWrap = 7,
        kRound = 8,
        kRoundDown = 9,
        kRoundUp = 10,
        kRangeToDb = 11,
        kRangeToInverseDb = 12,
        kAbsoluteValue = 13,
        kSine = 14,
        kCoSine = 15,
        kPower = 16,
        kScript = 17,
        kLastC = 18,
        kCustom = 100
    };

    FlowMathOp(Hmx::Object *);
    FlowMathOp(const FlowMathOp &other)
        : mValue(other.mValue), mType(other.mType), mCustom(other.mCustom),
          mPropertyPath(other.mPropertyPath), mPropertyProvider(other.mPropertyProvider) {
    }
    ~FlowMathOp();
    void Save(BinStream &);
    void Load(BinStream &, ObjectDir *);

    float Apply(float);

    float mValue; // 0x0
    MathOpTypes mType; // 0x4
    DataNode mCustom; // 0x8
    DataNode mPropertyPath; // 0x10
    FlowPtr<Hmx::Object> mPropertyProvider; // 0x18
};
