#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/TextFile.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/Option.h"
#include <map>

#define VAR_STACK_SIZE 100

std::map<Symbol, DataArray *> gMacroTable;
class ObjectDir *gDataDir;
Hmx::Object *gDataThis;
static bool gDataMacroWarning = true;
struct VarStack {
    void Set(DataNode *v) {
        var = v;
        value = *v;
    }

    DataNode *var;
    DataNode value;
};
VarStack gVarStack[VAR_STACK_SIZE];
VarStack *gVarStackPtr = gVarStack;

void DataMacroWarning(bool b) { gDataMacroWarning = b; }

Hmx::Object *DataThis() { return gDataThis; }

Loader *DataFactory(const FilePath &fp, LoaderPos pos) {
    return new DataLoader(fp, pos, true);
}

bool DataUpdateArray(DataArray *a1, DataArray *a2) {
    if (a2->Size() != 0) {
        if (a2->Type(0) == kDataSymbol) {
            if (a2->Sym(0) == a1->Sym(0)) {
                int size = a2->Size();
                a1->Resize(size);
                for (int i = 0; i < size; i++) {
                    a1->Node(i) = a2->Node(i);
                }
                return true;
            }
        }
        for (int i = 0; i < a2->Size(); i++) {
            if (a2->Type(i) == kDataArray) {
                if (DataUpdateArray(a1, a2->Array(i))) {
                    return true;
                }
            }
        }
    }
    return false;
}

void DataPushVar(DataNode *var) {
    gVarStackPtr++;
    MILO_ASSERT(gVarStackPtr - gVarStack < VAR_STACK_SIZE, 0x137);
    gVarStackPtr->Set(var);
}

void DataMergeTags(DataArray *dest, DataArray *src) {
    MILO_ASSERT(dest, 200);
    if (src && src != dest) {
        for (int i = 0; i < src->Size(); i++) {
            DataNode &node = src->Node(i);
            if (node.Type() == kDataArray) {
                DataArray *arr = node.UncheckedArray();
                if (arr->Size() != 0) {
                    DataArray *found = dest->FindArray(arr->UncheckedInt(0), false);
                    if (!found) {
                        dest->Resize(dest->Size() + 1);
                        const DataNode &destNode = arr;
                        dest->Node(dest->Size() - 1) = destNode;
                    } else
                        DataMergeTags(found, arr);
                }
            }
        }
    }
}

void DataReplaceTags(DataArray *dest, DataArray *src) {
    if (src != dest) {
        for (int i = 0; i < dest->Size(); i++) {
            DataNode &node = dest->Node(i);
            if (node.Type() == kDataArray) {
                DataArray *arr = node.UncheckedArray();
                if (arr->Size() != 0) {
                    DataArray *found = src->FindArray(arr->UncheckedInt(0), false);
                    if (found) {
                        DataReplaceTags(arr, found);
                        int inner_cnt = arr->Size();
                        found->Resize(inner_cnt);
                        for (int j = 0; j < inner_cnt; j++) {
                            found->Node(j) = arr->Node(j);
                        }
                        found->SetFileLine(arr->File(), arr->Line());
                        node = found;
                    }
                }
            }
        }
    }
}

Hmx::Object *DataSetThis(Hmx::Object *o) {
    if (o == gDataThis)
        return o;
    gDataDir = o ? o->DataDir() : ObjectDir::Main();
    Hmx::Object *old = gDataThis;
    gDataThis = o;
    static DataNode &thisVar = DataVariable("this");
    thisVar = o;
    o = old;
    return o;
}

void DataPopVar() {
    MILO_ASSERT(gVarStackPtr > gVarStack, 0x13E);
    *gVarStackPtr->var = gVarStackPtr->value;
    gVarStackPtr->value = 0;
    gVarStackPtr--;
}

void DataInit() {
    DataInitFuncs();
    TheLoadMgr.RegisterFactory("dta", DataFactory);
    TheLoadMgr.RegisterFactory("dtx", DataFactory);
    REGISTER_OBJ_FACTORY(TextFile);
    gDataMacroWarning = OptionBool("no_macro_warn", true);
    ObjectDir::PreInit(19997, 150000);
}

DataArray *DataGetMacro(Symbol s) {
    const std::map<Symbol, DataArray *>::iterator it = gMacroTable.find(s);
    if (it == gMacroTable.end())
        return 0;
    else
        return it->second;
}

Symbol DataGetMacroByInt(int value, const char *prefix) {
    FOREACH (it, gMacroTable) {
        DataArray *macro_array = it->second;
        if (macro_array->Size() != 0) {
            DataNode &node = macro_array->Node(0);
            if (node.Type() == kDataInt) {
                if (node.Int() == value) {
                    String name(it->first);
                    if (name.find(prefix) == 0) {
                        return it->first;
                    }
                }
            }
        }
    }
    return gNullStr;
}

void DataTerminate() {
    for (std::map<Symbol, DataArray *>::iterator i = gMacroTable.begin();
         i != gMacroTable.end();
         i++) {
        if (i->second) {
            i->second->Release();
            i->second = nullptr;
        }
    }
    gMacroTable.clear();
    gDataThis = nullptr;
    gDataDir = nullptr;
    DataTermFuncs();
}

void DataSetMacro(Symbol key, DataArray *macro) {
    DataArray *&val = gMacroTable[key];
    if (val)
        val->Release();
    if (macro) {
        if (val && gDataMacroWarning)
            MILO_NOTIFY(
                "Resetting macro %s (file %s, line %d)", key, macro->File(), macro->Line()
            );
        val = macro;
        macro->AddRef();
    } else
        val = nullptr;
}
