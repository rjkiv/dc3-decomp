#include "utl/Symbol.h"
#include "os/Debug.h"
#include "utl/KeylessHash.h"
#include "obj/DataFunc.h"
#include <vector>

StringTable *gStringTable;
static KeylessHash<const char *, const char *>
    gHashTable(0, (const char *)0, (const char *)-1, 0);

void Symbol::Terminate() {
    RELEASE(gStringTable);
    gHashTable.Clear();
}

void Symbol::PreInit(int stringSize, int hashSize) {
    gStringTable = new StringTable(stringSize);
    gHashTable.Resize(hashSize, nullptr);
    TheDebug.AddExitCallback(Symbol::Terminate);
}

Symbol::Symbol(const char *str) {
    if (str && *str) {
        const char **entry = gHashTable.Find(str);
        if (entry) {
            mStr = *entry;
        } else {
            mStr = gStringTable->Add(str);
            if (strlen(str) > 100) {
                MILO_NOTIFY("Huge symbol %s", str);
            }
            gHashTable.Insert(mStr);
        }
    } else
        mStr = gNullStr;
}

// not sure where else to put this, it's only used here
struct Alpha {
    bool operator()(const char *ci, const char *cj) { return strcmp(ci, cj) < 0; }
};

DataNode PrintSymbolTable(DataArray *msg) {
    MILO_LOG("Symbol table:\n");
    MILO_LOG("%d / %d hashes\n", gHashTable.NumEntries(), gHashTable.Size());
    MILO_LOG("%d / %d strings\n", gStringTable->UsedSize(), gStringTable->Size());
    MILO_LOG(
        "adding 30%%, suggest Symbol::PreInit(%d, %d)\n",
        (int)(gStringTable->UsedSize() * 1.3f),
        (int)((gHashTable.NumEntries() << 1) * 1.3f)
    );
    if (msg->Size() > 1) {
        std::vector<const char *> strs;
        strs.reserve(gHashTable.NumEntries());
        for (const char **it = gHashTable.Begin(); it != nullptr;
             it = gHashTable.Next(it)) {
            strs.push_back(*it);
        }
        std::sort(strs.begin(), strs.end(), Alpha());
        for (int i = 0; i < strs.size(); i++) {
            MILO_LOG("%s\n", strs[i]);
        }
    }
    return 0;
}

void Symbol::Init() {
    if (!gStringTable)
        PreInit(560000, 80000);
    DataRegisterFunc("print_symbol_table", PrintSymbolTable);
}
