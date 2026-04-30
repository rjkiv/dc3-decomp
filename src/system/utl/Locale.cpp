#include "utl/Locale.h"

#include "DataPointMgr.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Str.h"
#include "xdk/xbdm/xbdm.h"
#include <vector>

char gLocaleBuffers[4][50]; // not sure if these 2 should be globals but i currently have
int gLocalBufIdx = 0; // no other ideas

bool gShowTokensCheat;

DataNode DataSetLocaleVerboseNotify(DataArray *arr) {
    Locale::SetLocaleVerboseNotify(arr->Int(1));
    return DataNode(0);
}

DataNode DataToggleShowTokensCheat(DataArray *arr) {
    gShowTokensCheat = !gShowTokensCheat;
    return DataNode(0);
}

static int LocaleChunkSortFunc(const void *a, const void *b) {
    const LocaleChunkSort::OrderedLocaleChunk *chunkA =
        (const LocaleChunkSort::OrderedLocaleChunk *)a;
    const LocaleChunkSort::OrderedLocaleChunk *chunkB =
        (const LocaleChunkSort::OrderedLocaleChunk *)b;
    Symbol symA = chunkA->node1.LiteralSym(0);
    Symbol symB = chunkB->node1.LiteralSym(0);
    if (symA < symB)
        return -1;
    if (symA > symB)
        return 1;
    return chunkA->node2.Int(0) - chunkB->node2.Int(0);
}

void LocaleChunkSort::Sort(OrderedLocaleChunk *chunks, int count) {
    qsort(chunks, count, sizeof(OrderedLocaleChunk), LocaleChunkSortFunc);
}

void Locale::Terminate() {
    delete[] mSymTable;
    mSymTable = 0;
    delete[] mStrTable;
    mStrTable = 0;
    delete[] mUploadedFlags;
    mUploadedFlags = 0;
    RELEASE(mStringData);
    mSize = 0;
    mFile = Symbol();
    mNumFilesLoaded = 0;
}

void Locale::SetMagnuStrings(DataArray *da) {
    if (mMagnuStrings) {
        mMagnuStrings->Release();
        mMagnuStrings = 0;
    }
    mMagnuStrings = da;
}

void Locale::Init() {
    MILO_ASSERT(!mStrTable, 0x58);
    MILO_ASSERT(!mSymTable, 0x59);
    MILO_ASSERT(!mSize, 0x5A);
    MILO_ASSERT(!mStringData, 0x5B);
    MILO_ASSERT(!mNumFilesLoaded, 0x5C);

    mSize = 0;
    int i13 = 0;
    int numChunks = 0;
    LocaleChunkSort::OrderedLocaleChunk *chunks = 0;
    Symbol s60;

    // Check for alternate devkit locale file
    String devkitPath(FileMakePath(
        "devkit:\\locale", MakeString("%s\\locale_keep.dta", SystemLanguage())
    ));
    FileQualifiedFilename(devkitPath, devkitPath.c_str());

    static Symbol locale("locale");
    DataArrayPtr altCfg(DataNode(devkitPath), DataNode(locale));

    DataArray *cfg = SystemConfig();
    if (!cfg) {
        goto done;
    }

    cfg = SystemConfig("locale");

    if (DmMapDevkitDrive() >= 0) {
        if (FileExists(devkitPath.c_str(), 0, 0)) {
            MILO_NOTIFY("Using alternate locale file from%s", devkitPath);
            cfg = (DataArray *)altCfg;
        }
    }

    MemPushTemp();
    {
        std::vector<DataArray *> arrVec(cfg->Size() - 1);
        mNumFilesLoaded = arrVec.size();

        int i10 = 0;
        if (mInitialized) {
            for (int i = 1; i < cfg->Size(); i++) {
                const char *path = FileMakePath(FileGetPath(cfg->File()), cfg->Str(i));
                arrVec[i - 1] = DataReadFile(path, true);
                if (!arrVec[i - 1]) {
                    MILO_FAIL("could not load language file %s", path);
                }
                i10 += arrVec[i - 1]->Size();
            }

            chunks = new LocaleChunkSort::OrderedLocaleChunk[i10];

            numChunks = 0;
            for (int j = cfg->Size() - 2; j >= 0; j--) {
                DataArray *curArr = arrVec[j];
                for (int k = curArr->Size() - 1; k >= 0; k--, numChunks++) {
                    DataArray *chunkArr = curArr->Node(k).LiteralArray(curArr);
                    int size = chunkArr->Size();
                    if (size < 2) {
                        MILO_FAIL(
                            "%s line %d should have 2 entries, has %d, mismatched quotes?",
                            chunkArr->File(),
                            chunkArr->Line(),
                            size
                        );
                    }
                    chunks[numChunks].node1 = chunkArr->LiteralSym(0);
                    chunks[numChunks].node2 = numChunks;
                    chunks[numChunks].node3 = chunkArr->LiteralStr(1);
                }
                curArr->Release();
            }
        }

        if (cfg->Size() > 1) {
            LocaleChunkSort::Sort(chunks, numChunks);
        }

        mSize = 0;
        for (int i = 0; i < numChunks; i++) {
            Symbol curSym = chunks[i].node1.LiteralSym(0);
            if (curSym != s60) {
                const char *str = chunks[i].node3.LiteralStr(0);
                const char *p = str;
                while (*p++) {
                }
                i13 += (p - str) - 1;
                s60 = curSym;
                mSize++;
            }
        }
    }

    mSymTable = new Symbol[mSize];
    mStringData = new StringTable(i13);
    mStrTable = new const char *[mSize];
    mUploadedFlags = new bool[mSize];

    s60 = Symbol();

    if (numChunks > 0) {
        int chunkIdx = 0;
        for (int i = 0; i < numChunks; i++) {
            Symbol curSym = chunks[i].node1.LiteralSym(0);
            if (curSym != s60) {
                mUploadedFlags[chunkIdx] = 0;
                mSymTable[chunkIdx] = curSym;
                mStrTable[chunkIdx] = mStringData->Add(chunks[i].node3.LiteralStr(0));
                s60 = curSym;
                chunkIdx++;
            } else {
                MILO_WARN("Locale symbol '%s' redefined\n", curSym);
            }
        }
    }

    delete[] chunks;
    MemPopTemp();

done:
    if (cfg && cfg->Size() > 1) {
        mFile = cfg->Str(1);
    }

    DataRegisterFunc("set_locale_verbose_notify", DataSetLocaleVerboseNotify);
    DataRegisterFunc("toggle_show_tokens_cheat", DataToggleShowTokensCheat);
}

const char *Localize(Symbol sym, bool *found, Locale &loc) {
    if (gShowTokensCheat) {
        if (found) {
            *found = true;
        }
        return sym.Str();
    } else {
        const char *res = loc.Localize(sym, false);
        bool is_found = (res != nullptr);
        if (!is_found) {
            res = sym.Str();
            Locale::sIgnoreMissingText = sym.Str();

            if (Locale::GetLocaleVerboseNotify()) {
                MILO_NOTIFY("\"%s\" needs localization", sym);
            }
        }
        if (found) {
            *found = is_found;
        }

        return res;
    }
}

const char *LocalizeFloat(const char *fmt, float num) {
    const char *str = MakeString<float>(fmt, num);
    static Symbol locale_decimal_separator("locale_decimal_separator");
    const char *sep = TheLocale.Localize(locale_decimal_separator, false);

    if (sep != nullptr && *sep != '.') {
        char *dest = gLocaleBuffers[gLocalBufIdx];

        strncpy(dest, str, 50);
        dest[49] = '\0';

        for (char *ptr = dest; *ptr != '\0'; ++ptr) {
            if (*ptr == '.') {
                *ptr = *sep;
                break;
            }
        }

        gLocalBufIdx = (gLocalBufIdx + 1) % 4;

        return dest;
    }
    return str;
}

const char *Locale::Localize(Symbol sym, bool param_2) const {
    if (sym.Str() == gNullStr) {
        return "";
    }
    MILO_ASSERT(!mSymTable, 0x1d8);

    static Symbol eng("eng");

    if (mMagnuStrings) {
        if (SystemLanguage() == eng) {
            DataArray *arr = mMagnuStrings->FindArray(sym, false);
            if (arr) {
                return arr->Node(1).Str(arr);
            }
        }
    }
    { // if we dont scope index it uses 0x54 instead of 0x50 lol
        int index;
        if (FindDataIndex(sym, index, param_2)) {
            return mStrTable[index];
        }
    }

    if (UsingCD()) {
        SendDebugDataPoint<const char *, Symbol, const char *, bool>(
            "debug/locale/token", "token", sym, "success", false
        );
    }

    return nullptr;
}

bool Locale::FindDataIndex(Symbol sym, int &index, bool fail) const {
    int low = 0;
    int high = mSize - 1;

    while (high - low >= 0) {
        int mid = (low + high) >> 1;
        Symbol midSym = mSymTable[mid];

        if (sym > midSym) {
            low = mid + 1;
        } else if ((int)sym < (int)midSym) {
            high = mid - 1;
        } else {
            index = mid;
            return true;
        }
    }

    if (fail) {
        MILO_FAIL("Couldn't find '%s' in array (file %s)", sym.Str(), mFile.Str());
    }

    return false;
}