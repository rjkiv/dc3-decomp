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
    bs << unk_0x4;
    bs << unk_0x0;
    bs << unk_0x18;
    bs << rhs;
    bs << lhs;
END_SAVES

FlowMathOp::FlowMathOp(Hmx::Object *obj)
    : unk_0x0(0.0f), unk_0x4(0), unk_0x18(obj, nullptr) {}

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
    d >> unk_0x4;
    if (d.rev < 1 && unk_0x4 == 8) {
        unk_0x4 = -1;
    }
    if (d.rev < 2) {
        if (unk_0x4 == 4) {
            unk_0x4 = 8;
        } else if (unk_0x4 > 4) {
            unk_0x4--;
        }
    }
    if (d.rev < 3 && unk_0x4 > 2) {
        unk_0x4++;
    }
    d >> unk_0x0;
    if (d.rev < 6) {
        bool b;
        d >> b;
        unk_0x18 = b ? FlowNode::LoadObjectFromMainOrDir(bs, dir) : nullptr;
        d >> rhs;
        Symbol s;
        if (d.rev > 3) {
            d >> s;
        }
        if (d.rev > 4) {
            d >> lhs;
        }
    } else {
        d >> unk_0x18;
        d >> rhs;
        d >> lhs;
    }
}

float FlowMathOp::Apply(float f1) {
    float y = unk_0x0;
    if (unk_0x18 && rhs.Type() == kDataArray) {
        const DataNode *prop = unk_0x18->Property(rhs.Array(), false);
        if (prop && prop->CompatibleType(kDataFloat)) {
            y = prop->LiteralFloat();
        }
    }
    switch (unk_0x4) {
    case 0x11: {
        if (lhs.Type() == kDataString) {
            String str = lhs.Str();
            DataNode n;
            if (!str.empty()) {
                DataVariable("val") = f1;
                float ret = f1;
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
                        lhs.Str()
                    );
                }
                return ret;
            }
        } else {
            goto script;
        }
        return f1;
    }
    case 0:
        return f1 + y;
    case 1:
        return f1 - y;
    case 2:
        return f1 * y;
    case 3:
        if (y == 0) {
            y = 0.0001f;
        }
        return f1 / y;
    case 4:
        return RandomFloat(0, y * 2) - y + f1;
    case 5:
        if (f1 >= y) {
            return f1;
        } else {
            return y;
        }
    case -1:
        return y;
    case 6:
        if (f1 <= y) {
            return f1;
        } else {
            return y;
        }
    case 7:
        if (y <= 0) {
            return f1;
        } else {
            return fmod(f1, y);
        }
    case 8: {
        if (y == 0) {
            y = 1;
        }
        int num = (((y / 2) + f1) / y);
        return (float)num * y;
    }
    case 9:
        if (y <= 0) {
            y = 1;
        }
        return floorf(f1 / y) * y;
    case 10:
        if (y <= 0) {
            y = 1;
        }
        return ceilf(f1 / y) * y;
    case 11: {
        float clamped = Clamp(0.0f, y, f1);
        if (y != 0) {
            clamped = clamped / y;
        }
        return RatioToDb(clamped);
    }
    case 12: {
        float clamped = Clamp(0.0f, y, f1);
        if (y != 0) {
            clamped = clamped / y;
        }
        return RatioToDb(1.0f - clamped);
    }
    case 13:
        return fabs(f1);
    case 14:
        return sin(f1);
    case 15:
        return cos(f1);
    case 16:
        return pow(f1, y);
    case 100:
    script:
        if (lhs.Type() == kDataSymbol) {
            DataArray *cfg = SystemConfig("objects", "FlowNode", "mathops");
            DataArray *mathOpArray = cfg->FindArray(lhs.Sym(), false);
            if (mathOpArray) {
                DataArray *scriptArr = mathOpArray->FindArray("script");
                DataVariable("val") = f1;
                DataVariable("prop_val") = y;
                return scriptArr->Float(1);
            }
        }
        return f1;
    default:
        MILO_NOTIFY_ONCE("Bad mathop operation value");
        return f1;
    }
}
