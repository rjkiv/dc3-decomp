#include "flow/DrivenPropertyMathOps.h"
#include "flow/FlowNode.h"
#include "math/Decibels.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/BinStream.h"

FlowMathOp::~FlowMathOp() {}

BEGIN_SAVES(FlowMathOp)
    SAVE_REVS(6, 0)
    bs << mType;
    bs << mValue;
    bs << mPropertyProvider;
    bs << mPropertyPath;
    bs << mCustom;
END_SAVES

FlowMathOp::FlowMathOp(Hmx::Object *obj)
    : mValue(0.0f), mType(kAdd), mPropertyProvider(obj, nullptr) {}

INIT_REVS(6, 0)

void FlowMathOp::Load(BinStream &bs, ObjectDir *dir) {
    LOAD_REVS(bs)
    if (d.rev > 6) {
        MILO_FAIL(
            "%s can't load new %s version %d > %d",
            PathName(dir),
            "FlowMathOp",
            d.rev,
            gRev
        );
    }
    if (d.altRev > 0) {
        MILO_FAIL(
            "%s can't load new %s alt version %d > %d",
            PathName(dir),
            "FlowMathOp",
            d.altRev,
            gAltRev
        );
    }
    d >> (int &)mType;
    if (d.rev < 1 && mType == kRound) {
        mType = kInitValue;
    }
    if (d.rev < 2) {
        if (mType == kVariance) {
            mType = kRound;
        } else if (mType > kVariance) {
            mType = (MathOpTypes)((int)mType - 1);
        }
    }
    if (d.rev < 3 && mType > kMultiply) {
        mType = (MathOpTypes)((int)mType + 1);
    }
    d >> mValue;
    if (d.rev < 6) {
        bool b;
        d >> b;
        mPropertyProvider = b ? FlowNode::LoadObjectFromMainOrDir(bs, dir) : nullptr;
        d >> mPropertyPath;
        Symbol s;
        if (d.rev > 3) {
            d >> s;
        }
        if (d.rev > 4) {
            d >> mCustom;
        }
    } else {
        d >> mPropertyProvider;
        d >> mPropertyPath;
        d >> mCustom;
    }
}

float FlowMathOp::Apply(float val) {
    float y = mValue;
    if (mPropertyProvider && mPropertyPath.Type() == kDataArray) {
        const DataNode *prop = mPropertyProvider->Property(mPropertyPath.Array(), false);
        if (prop && prop->CompatibleType(kDataFloat)) {
            y = prop->LiteralFloat();
        }
    }
    switch (mType) {
    case kScript: {
        if (mCustom.Type() == kDataString) {
            String str = mCustom.Str();
            DataNode n;
            if (str.empty()) {
                return val;
            } else {
                DataVariable("val") = val;
                float ret = val;
                MILO_TRY {
                    n = DataReadString(str.c_str());
                    n.Array()->Release();
                    if (n.Array()->Type(0) == kDataCommand && n.Array()->Size() == 1) {
                        ret = n.Array()->Command(0)->Execute().Float();
                    } else {
                        ret = n.Array()->Execute().Float();
                    }
                }
                MILO_CATCH(msg) {
                    MILO_NOTIFY(
                        "Bad script expression in mathop : %s, expression is: %s",
                        msg,
                        mCustom.Str()
                    );
                }
                return ret;
            }
        } else {
            goto script;
        }
        return val;
    }
    case kAdd:
        return val + y;
    case kSubtract:
        return val - y;
    case kMultiply:
        return val * y;
    case kDivide:
        if (y == 0) {
            y = 0.0001f;
        }
        return val / y;
    case kVariance:
        return RandomFloat(0, y * 2) - y + val;
    case kClampMin:
        if (val >= y) {
            return val;
        } else {
            return y;
        }
    case kInitValue:
        return y;
    case kClampMax:
        if (val <= y) {
            return val;
        } else {
            return y;
        }
    case kWrap:
        if (y <= 0) {
            return val;
        } else {
            return fmod(val, y);
        }
    case kRound: {
        if (y == 0) {
            y = 1;
        }
        int num = (((y / 2) + val) / y);
        return (float)num * y;
    }
    case kRoundDown:
        if (y <= 0) {
            y = 1;
        }
        return floorf(val / y) * y;
    case kRoundUp:
        if (y <= 0) {
            y = 1;
        }
        return ceilf(val / y) * y;
    case kRangeToDb: {
        float clamped = Clamp(0.0f, y, val);
        if (y != 0) {
            clamped = clamped / y;
        }
        return RatioToDb(clamped);
    }
    case kRangeToInverseDb: {
        float clamped = Clamp(0.0f, y, val);
        if (y != 0) {
            clamped = clamped / y;
        }
        return RatioToDb(1.0f - clamped);
    }
    case kAbsoluteValue:
        return fabs(val);
    case kSine:
        return sin(val);
    case kCoSine:
        return cos(val);
    case kPower:
        return pow(val, y);
    case kCustom:
    script:
        if (mCustom.Type() == kDataSymbol) {
            DataArray *cfg = SystemConfig("objects", "FlowNode", "mathops");
            DataArray *mathOpArray = cfg->FindArray(mCustom.Sym(), false);
            if (mathOpArray) {
                DataArray *scriptArr = mathOpArray->FindArray("script");
                DataVariable("val") = val;
                DataVariable("prop_val") = y;
                return scriptArr->Float(1);
            }
        }
        return val;
    default:
        MILO_NOTIFY_ONCE("Bad mathop operation value");
        return val;
    }
}
