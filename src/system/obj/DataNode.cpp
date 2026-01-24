#include "Data.h"
#include "Dir.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Str.h"
#include "utl/TextStream.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "obj/DataUtl.h"
#include <cstring>
#include <map>

int gEvalIndex;
std::map<Symbol, DataNode> gDataVars;
DataNode gEvalNode[8];

bool DataNode::CompatibleType(DataType ty) const {
    if (mType == ty)
        return true;
    switch (mType) {
    case kDataInt:
        return ty == kDataFloat;
    case kDataSymbol:
        return ty == kDataString || ty == kDataObject;
    case kDataString:
        return ty == kDataObject;
    default:
        return false;
    }
    return true;
}

const char *DataNode::DataTypeString(DataType ty) {
    switch (ty) {
    case kDataInt:
        return "kDataInt";
    case kDataFloat:
        return "kDataFloat";
    case kDataVar:
        return "kDataVar";
    case kDataFunc:
        return "kDataFunc";
    case kDataObject:
        return "kDataObject";
    case kDataSymbol:
        return "kDataSymbol";
    case kDataUnhandled:
        return "kDataUnhandled";
    case kDataArray:
        return "kDataArray";
    case kDataCommand:
        return "kDataCommand";
    case kDataString:
        return "kDataString";
    case kDataProperty:
        return "kDataProperty";
    case kDataGlob:
        return "kDataGlob";
    case kDataIfdef:
        return "kDataIfdef";
    case kDataElse:
        return "kDataElse";
    case kDataEndif:
        return "kDataEndif";
    case kDataDefine:
        return "kDataDefine";
    case kDataInclude:
        return "kDataInclude";
    case kDataMerge:
        return "kDataMerge";
    case kDataIfndef:
        return "kDataIfndef";
    case kDataAutorun:
        return "kDataAutorun";
    case kDataUndef:
        return "kDataUndef";
    default:
        return "Unknown data type";
    }
}

DataNode::DataNode(const char *c) {
    mValue.array = new DataArray(c, strlen(c) + 1);
    mType = kDataString;
}

DataNode::DataNode(const String &str) {
    mValue.array = new DataArray(str.c_str(), str.length() + 1);
    mType = kDataString;
}

DataNode::DataNode(const DataNode &node) {
    mValue = node.mValue;
    mType = node.mType;
    if (mType & kDataArray)
        mValue.array->AddRef();
}

DataNode::DataNode(const DataArrayPtr &ptr) {
    mValue.array = ptr;
    ptr->AddRef();
    mType = kDataArray;
}

DataNode::DataNode(DataArray *array, DataType type) {
    MILO_ASSERT(array, 0x158);
    mValue.array = array;
    mValue.array->AddRef();
    MILO_ASSERT(type & kDataArray, 0x15B);
    mType = type;
}

DataNode &DataNode::operator=(const DataNode &node) {
    if (&node != this) {
        if (mType & kDataArray)
            mValue.array->Release();
        mValue = node.mValue;
        mType = node.mType;
        if (mType & kDataArray)
            mValue.array->AddRef();
    }
    return *this;
}

const DataNode &UseQueue(const DataNode &node) {
    int i;
    gEvalNode[gEvalIndex] = node;
    i = gEvalIndex;
    gEvalIndex = gEvalIndex + 1 & 7;
    return gEvalNode[i];
}

const DataNode &DataNode::Evaluate() const {
    if (mType == kDataCommand) {
        return UseQueue(mValue.array->Execute(true));
    } else if (mType == kDataVar) {
        return *mValue.var;
    } else if (mType == kDataProperty) {
        MILO_ASSERT(gDataThis, 0x7A);
        const DataNode *n = gDataThis->Property(mValue.array, true);
        return UseQueue(*n);
    } else
        return *this;
}

bool DataNode::NotNull() const {
    const DataNode &n = Evaluate();
    DataType t = n.Type();
    if (t == kDataSymbol) {
        return n.mValue.symbol[0] != 0;
    } else if (t == kDataString) {
        return n.mValue.array->Size() < -1;
    } else if (t == kDataGlob) {
        return n.mValue.array->Size() & -1;
    } else
        return n.mValue.array != 0;
}

bool DataVarExists(Symbol s) { return gDataVars.find(s) != gDataVars.end(); }

const char *DataVarName(const DataNode *var) {
    FOREACH (it, gDataVars) {
        if ((&it->second) == var) {
            return it->first.Str();
        }
    }
    return "<null>";
}

inline bool HasChar(const char *str, char c) {
    while (*str != '\0') {
        if (*str++ == c)
            return true;
    }
    return false;
}

void DataNode::Print(TextStream &ts, bool b, int i3) const {
    switch (mType) {
    case kDataUnhandled:
        ts << "kDataUnhandled";
        break;
    case kDataInt:
        ts << mValue.integer;
        break;
    case kDataString:
        if (b)
            ts << mValue.var->mValue.symbol;
        else {
            ts << "\"";
            char *tok = strtok((char *)mValue.var->mValue.symbol, "\"");
            while (tok) {
                ts << tok;
                tok = strtok(0, "\"");
                if (tok) {
                    ts << "\\q";
                    tok[-1] = '\"';
                }
            }
            ts << "\"";
        }
        break;
    case kDataSymbol:
        if (!HasChar(mValue.symbol, ' ') && !HasChar(mValue.symbol, '#')
            && !HasChar(mValue.symbol, '-'))
            ts << mValue.symbol;
        else
            ts << "'" << mValue.symbol << "'";
        break;
    case kDataGlob:
        ts << "<glob " << -mValue.array->Size() << ">";
        break;
    case kDataFloat:
        ts << mValue.real;
        break;
    case kDataArray:
    case kDataCommand:
    case kDataProperty:
        mValue.array->Print(ts, mType, b, i3);
        break;
    case kDataObject:
        ts << mValue.object;
        break;
    case kDataVar:
        ts << '$' << DataVarName(mValue.var);
        break;
    case kDataFunc:
        ts << DataFuncName(mValue.func);
        break;
    case kDataDefine:
        ts << "\n#define " << mValue.symbol << "\n";
        break;
    case kDataUndef:
        ts << "\n#undef " << mValue.symbol << "\n";
        break;
    case kDataIfdef:
        ts << "\n#ifdef " << mValue.symbol << "\n";
        break;
    case kDataIfndef:
        ts << "\n#ifndef " << mValue.symbol << "\n";
        break;
    case kDataAutorun:
        ts << "\n#autorun\n";
        break;
    case kDataElse:
        ts << "\n#else\n";
        break;
    case kDataEndif:
        ts << "\n#endif\n";
        break;
    case kDataInclude:
        ts << "\n#include " << mValue.symbol << "\n";
        break;
    case kDataMerge:
        ts << "\n#merge " << mValue.symbol << "\n";
        break;
    }
}

bool DataNode::PrintUnused(TextStream &, bool) const {
    MILO_NOTIFY("Enable PRINT_UNUSED in Data.h to utilize DataNode::PrintUnused()");
    return false;
}

void DataNode::Save(BinStream &d) const {
    d << mType;
    switch (mType) {
    case kDataSymbol:
    case kDataIfdef:
    case kDataDefine:
    case kDataInclude:
    case kDataMerge:
    case kDataIfndef:
    case kDataUndef:
        d << mValue.symbol;
        break;
    case kDataFloat:
        d << mValue.real;
        break;
    case kDataString:
    case kDataGlob:
        mValue.array->SaveGlob(d, mType == kDataString);
        break;
    case kDataArray:
    case kDataCommand:
    case kDataProperty:
        mValue.array->Save(d);
        break;
    case kDataObject:
        if (mValue.object)
            d << mValue.object->Name();
        else
            d << "";
        break;
    case kDataVar:
        d << DataVarName(mValue.var);
        break;
    case kDataFunc:
        d << DataFuncName(mValue.func);
        break;
    case kDataInt:
    case kDataUnhandled:
    case kDataElse:
    case kDataEndif:
    case kDataAutorun:
        d << mValue.integer;
        break;
    default:
        MILO_FAIL("Unrecognized node type: %x", mType);
        break;
    }
}

int DataNode::Int(const DataArray *source) const {
    const DataNode &n = Evaluate();
    if (n.mType != kDataInt) {
        String s;
        n.Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Int (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Int", s);
    }
    return n.mValue.integer;
}

int DataNode::LiteralInt(const DataArray *source) const {
    if (mType != kDataInt) {
        String s;
        Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Int (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Int", s);
    }
    return mValue.integer;
}

Symbol DataNode::Sym(const DataArray *source) const {
    const DataNode &n = Evaluate();
    if (n.mType != kDataSymbol) {
        String s;
        n.Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Symbol (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Symbol", s);
    }
    return STR_TO_SYM(n.mValue.symbol);
}

Symbol DataNode::LiteralSym(const DataArray *source) const {
    if (mType != kDataSymbol) {
        String s;
        Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Symbol (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Symbol", s);
    }
    return STR_TO_SYM(mValue.symbol);
}

Symbol DataNode::ForceSym(const DataArray *source) const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataSymbol) {
        return STR_TO_SYM(n.mValue.symbol);
    } else {
        if (n.mType != kDataString) {
            String s;
            n.Print(s, true, 0);
            if (source)
                MILO_FAIL(
                    "Data %s is not String (file %s, line %d)",
                    s.c_str(),
                    source->File(),
                    source->Line()
                );
            else
                MILO_FAIL("Data %s is not String", s);
        }
        return Symbol(n.mValue.var->mValue.symbol);
    }
}

const char *DataNode::Str(const DataArray *source) const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataSymbol) {
        return n.mValue.symbol;
    } else {
        if (n.mType != kDataString) {
            String s;
            n.Print(s, true, 0);
            if (source)
                MILO_FAIL(
                    "Data %s is not String (file %s, line %d)",
                    s.c_str(),
                    source->File(),
                    source->Line()
                );
            else
                MILO_FAIL("Data %s is not String", s);
        }
        return n.mValue.var->mValue.symbol;
    }
}

const char *DataNode::LiteralStr(const DataArray *source) const {
    if (mType == kDataSymbol) {
        return mValue.symbol;
    } else {
        if (mType != kDataString) {
            String s;
            Print(s, true, 0);
            if (source)
                MILO_FAIL(
                    "Data %s is not String (file %s, line %d)",
                    s.c_str(),
                    source->File(),
                    source->Line()
                );
            else
                MILO_FAIL("Data %s is not String", s);
        }
        return mValue.var->mValue.symbol;
    }
}

float DataNode::Float(const DataArray *source) const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataInt) {
        return n.mValue.integer;
    } else {
        if (n.mType != kDataFloat) {
            String s;
            n.Print(s, true, 0);
            if (source)
                MILO_FAIL(
                    "Data %s is not Float (file %s, line %d)",
                    s.c_str(),
                    source->File(),
                    source->Line()
                );
            else
                MILO_FAIL("Data %s is not Float", s);
        }
        return n.mValue.real;
    }
}

float DataNode::LiteralFloat(const DataArray *source) const {
    if (mType == kDataInt) {
        return mValue.integer;
    } else {
        if (mType != kDataFloat) {
            String s;
            Print(s, true, 0);
            if (source)
                MILO_FAIL(
                    "Data %s is not Float (file %s, line %d)",
                    s.c_str(),
                    source->File(),
                    source->Line()
                );
            else
                MILO_FAIL("Data %s is not Float", s);
        }
        return mValue.real;
    }
}

DataFunc *DataNode::Func(const DataArray *source) const {
    if (mType != kDataFunc) {
        String s;
        Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Func (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Func", s);
    }
    return mValue.func;
}

Hmx::Object *DataNode::GetObj(const DataArray *source) const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataObject)
        return n.mValue.object;
    else {
        const char *str = n.LiteralStr(source);
        Hmx::Object *ret = 0;
        if (*str != '\0') {
            ret = gDataDir->FindObject(str, true, true);
            if (!ret) {
                const char *msg =
                    PathName(gDataDir) != nullptr ? PathName(gDataDir) : "**no file**";
                MILO_FAIL(kNotObjectMsg, str, msg);
            }
        }
        return ret;
    }
}

DataArray *DataNode::Array(const DataArray *source) const {
    const DataNode &n = Evaluate();
    if (n.mType != kDataArray) {
        String s;
        n.Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Array (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Array", s);
    }
    return n.mValue.array;
}

DataArray *DataNode::LiteralArray(const DataArray *source) const {
    if (mType != kDataArray) {
        String s;
        Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Array (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Array", s);
    }
    return mValue.array;
}

DataArray *DataNode::Command(const DataArray *source) const {
    if (mType != kDataCommand) {
        String s;
        Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Command (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Command", s);
    }
    return mValue.array;
}

DataNode *DataNode::Var(const DataArray *source) const {
    if (mType != kDataVar) {
        String s;
        Print(s, true, 0);
        if (source)
            MILO_FAIL(
                "Data %s is not Var (file %s, line %d)",
                s.c_str(),
                source->File(),
                source->Line()
            );
        else
            MILO_FAIL("Data %s is not Var", s);
    }
    return mValue.var;
}

bool DataNode::operator>(const DataNode &other) const {
    if ((mType == kDataInt || mType == kDataFloat)
        && (other.mType == kDataInt || other.mType == kDataFloat)) {
        return LiteralFloat() > other.LiteralFloat();
    } else
        return false;
}

bool DataNode::Equal(const DataNode &n, DataArray *a, bool warn) const {
    const DataNode &first = mType < n.Type() ? *this : n;
    const DataNode &second = mType < n.Type() ? n : *this;
    DataType firstType = first.Type();
    DataType secondType = second.Type();
    if (firstType == secondType) {
        bool res;
        if (firstType == kDataString) {
            res = streq(first.UncheckedStr(), second.UncheckedStr());
        } else {
            res = first.UncheckedInt() == second.UncheckedInt();
        }
        return res;
    } else {
        const char *objName = "";
        if (firstType == kDataInt && secondType == kDataFloat) {
            return first.UncheckedInt() == second.UncheckedInt();
        } else {
            if (firstType == kDataObject) {
                Hmx::Object *obj = first.UncheckedObj();
                if (obj)
                    objName = obj->Name();
                if (secondType == kDataSymbol) {
                    return streq(objName, second.UncheckedStr());
                } else if (secondType == kDataString) {
                    return streq(objName, second.UncheckedStr());
                }
            }
            if (firstType == kDataSymbol) {
                if (secondType == kDataString) {
                    return streq(first.UncheckedStr(), second.UncheckedStr());
                }
            } else if (secondType != kDataString && secondType != kDataSymbol) {
                warn &= secondType != kDataObject; // i dunno lol
            }
        }
        if (firstType == kDataUnhandled || secondType == kDataUnhandled) {
            warn = false;
        }
        if (warn) {
            StackString<32> str1;
            StackString<32> str2;
            first.Print(str1, true, 0);
            second.Print(str2, true, 0);
            MILO_NOTIFY_ONCE(
                "DataNode::Equal: DataNodes %s and %s (%s and %s) are not compatible (file %s, line %d)",
                str1,
                str2,
                DataTypeString(first.Type()),
                DataTypeString(second.Type()),
                a ? a->File() : "",
                a ? a->Line() : -1
            );
        }
    }
    return false;
}

bool DataNode::operator!=(const DataNode &other) const {
    return !Equal(other, nullptr, true);
}

DataNode &DataVariable(Symbol s) { return gDataVars[s]; }

void DataNode::Load(BinStream &d) {
    static char buf[128];
    d >> (int &)mType;
    switch (mType) {
    case kDataFunc: {
        Symbol sym;
        d >> sym;
        const std::map<Symbol, DataFunc *>::iterator it = gDataFuncs.find(sym);
        if (it == gDataFuncs.end()) {
            MILO_FAIL("Couldn't bind %s", sym);
        }
        mValue.func = it->second;
        break;
    }
    case kDataSymbol:
    case kDataIfdef:
    case kDataDefine:
    case kDataInclude:
    case kDataMerge:
    case kDataIfndef:
    case kDataUndef: {
        Symbol sym;
        d >> sym;
        mValue.symbol = sym.Str();
        break;
    }
    case kDataFloat:
        d >> mValue.real;
        break;
    case kDataString:
    case kDataGlob:
        mValue.array = new DataArray(0);
        mValue.array->LoadGlob(d, mType == kDataString);
        break;
    case kDataArray:
    case kDataCommand:
    case kDataProperty:
        mValue.array = new DataArray(0);
        mValue.array->Load(d);
        break;
    case kDataObject:
        d.ReadString(buf, 0x80);
        mValue.object = gDataDir->FindObject(buf, true, true);
        if (!mValue.object && *buf) {
            MILO_NOTIFY("Couldn't find %s from %s", buf, gDataDir->Name());
        }
        break;
    case kDataVar: {
        Symbol sym;
        d >> sym;
        Symbol key = sym;
        mValue.var = &gDataVars[key];
        break;
    }
    case kDataUnhandled:
    case kDataInt:
    case kDataElse:
    case kDataEndif:
    case kDataAutorun:
        d >> mValue.integer;
        break;
    default:
        MILO_FAIL("Unrecognized node type: %x", mType);
        break;
    }
}

void DataNode::Load(BinStream &d, ObjectDir *dir) {
    ObjectDir *old = gDataDir;
    gDataDir = dir;
    Load(d);
    gDataDir = old;
}
