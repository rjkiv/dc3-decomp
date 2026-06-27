#include "utl/Locale.h"
#include "DataPointMgr.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/LocaleChunkSort.h"
#include "utl/Str.h"
#include "utl/StringTable.h"
#include "xdk/xbdm/xbdm.h"
#include <vector>

static char gFloatBufs[4][50];
static char gSepIntBufs[4][50];
const char *Locale::sIgnoreMissingText = nullptr;
bool Locale::sVerboseNotify = false;
bool gShowTokensCheat = false;
static int gNextSepIntBuf = 0;
static int gNextFloatBuf = 0;
Locale TheLocale;

void SyncReloadLocale() {
    static Symbol locale("locale");
    DataArray *cfg = SystemConfig(locale);
    for (int i = 1; i < cfg->Size(); i++) {
        const char *str = cfg->Str(i);
        const char *path = FileMakePath(FileGetPath(cfg->File()), str);
        const char *msg;
        if (SystemExec(MakeString("p4 sync %s", path)) == 0) {
            MILO_LOG("updated %s\n", path);
        } else {
            MILO_LOG("failed to update %s\n", path);
        }
    }
    TheLocale.Terminate();
    TheLocale.Init();
}

DataNode DataSetLocaleVerboseNotify(DataArray *arr) {
    Locale::SetLocaleVerboseNotify(arr->Int(1));
    return 0;
}

DataNode DataToggleShowTokensCheat(DataArray *arr) {
    gShowTokensCheat = !gShowTokensCheat;
    return 0;
}

const char *Localize(Symbol token, bool *success, Locale &locale) {
    if (gShowTokensCheat) {
        if (success) {
            *success = true;
        }
        return token.Str();
    } else {
        const char *res = locale.Localize(token, false);
        bool is_found = (res != nullptr);
        if (!is_found) {
            res = token.Str();
            Locale::sIgnoreMissingText = token.Str();

            if (Locale::GetLocaleVerboseNotify()) {
                MILO_NOTIFY("\"%s\" needs localization", token);
            }
        }
        if (success) {
            *success = is_found;
        }

        return res;
    }
}

const char *LocalizeSeparatedInt(int num, Locale &locale) {
    static Symbol locale_separator("locale_separator");
    bool success = false;
    char numChar[2];
    const char *loc = Localize(locale_separator, &success, locale);
    if (!success) {
        loc = ",";
    }
    if (streq(loc, gNullStr)) {
        return MakeString("%i", num);
    } else {
        int len = strlen(loc);
        char *curBuf = gSepIntBufs[gNextSepIntBuf];
        curBuf[49] = '\0';
        int i9 = 49;
        bool neg = num < 0;
        if (neg) {
            num = abs(num);
        }
        for (int i8 = 0; i8 == 0 || num > 0; i8++, num /= 10) {
            if (i8 % 3 == 0 && i8 > 0) {
                for (int i = len - 1; i >= 0; i--) {
                    curBuf[--i9] = loc[i];
                }
            }
            Hx_snprintf(numChar, sizeof(numChar), "%d", num % 10);
            curBuf[--i9] = numChar[0];
        }
        if (neg) {
            curBuf[--i9] = '-';
        }
        char *ret = &curBuf[i9];
        gNextSepIntBuf = (gNextSepIntBuf + 1) % 4;
        return ret;
    }
}

const char *LocalizeFloat(const char *fmt, float num) {
    const char *str = MakeString<float>(fmt, num);
    static Symbol locale_decimal_separator("locale_decimal_separator");
    const char *sep = TheLocale.Localize(locale_decimal_separator, false);

    if (sep != nullptr && *sep != '.') {
        char *dest = gFloatBufs[gNextFloatBuf];

        strncpy(dest, str, 50);
        dest[49] = '\0';

        for (char *ptr = dest; *ptr != '\0'; ++ptr) {
            if (*ptr == '.') {
                *ptr = *sep;
                break;
            }
        }

        gNextFloatBuf = (gNextFloatBuf + 1) % 4;

        return dest;
    }
    return str;
}

void Locale::Init() {
    MILO_ASSERT(!mStrTable, 0x58);
    MILO_ASSERT(!mSymTable, 0x59);
    MILO_ASSERT(!mSize, 0x5A);
    MILO_ASSERT(!mStringData, 0x5B);
    MILO_ASSERT(!mNumFilesLoaded, 0x5C);
    mSize = 0;
    Symbol s21;
    int strtablesize = 0;
    int numNodesTotal = 0;
    int numElements = 0;
    LocaleChunkSort::OrderedLocaleChunk *chunks = nullptr;
    DataArray *localeCfg = nullptr;
    String stre4 = FileMakePath(
        "devkit:\\locale", MakeString("%s\\locale_keep.dta", SystemLanguage())
    );
    const char *old;
    FileQualifiedFilename(stre4, old);
    static Symbol locale("locale");
    DataArrayPtr ptr(locale, stre4);

    if (SystemConfig()) {
        localeCfg = SystemConfig("locale");
        if (DmMapDevkitDrive() >= 0 && FileExists(old, 0, nullptr)) {
            MILO_NOTIFY("Using alternate locale file from HDD: %s", stre4);
            localeCfg = ptr;
        }
        {
            MemDoTempAllocations t;
            std::vector<DataArray *> arrays(localeCfg->Size() - 1);
            mNumFilesLoaded = arrays.size();
            if (mInitialized) {
                for (int i = 1; i < localeCfg->Size(); i++) {
                    const char *curStr = localeCfg->Str(i);
                    const char *curPath =
                        FileMakePath(FileGetPath(localeCfg->File()), curStr);
                    arrays[i - 1] = DataReadFile(curPath, true);
                    if (!arrays[i - 1]) {
                        MILO_FAIL("could not load language file %s", curPath);
                    }
                    numNodesTotal += arrays[i - 1]->Size();
                }
                chunks = new LocaleChunkSort::OrderedLocaleChunk[numNodesTotal];
                numElements = 0;
                for (int i = localeCfg->Size() - 2; i >= 0; i--) {
                    DataArray *arr = arrays[i];
                    for (int j = arr->Size() - 1; j >= 0; j--) {
                        DataArray *curArr = arr->LiteralArray(j);
                        if (curArr->Size() < 2) {
                            MILO_FAIL(
                                "%s line %d should have 2 entries, has %d, mismatched quotes?",
                                curArr->File(),
                                curArr->Line(),
                                curArr->Size()
                            );
                        }
                        chunks[numElements].sym = curArr->LiteralSym(0);
                        chunks[numElements].pos = numElements;
                        chunks[numElements].str = curArr->LiteralStr(1);
                        numElements++;
                    }
                    arr->Release();
                }
                if (localeCfg->Size() > 1) {
                    LocaleChunkSort sort;
                    sort.Sort(chunks, numElements);
                }
                mSize = 0;
                for (int i = 0; i < numElements; i++) {
                    Symbol sym = chunks[i].sym.LiteralSym();
                    if (sym != s21) {
                        int len = strlen(chunks[i].str.LiteralStr());
                        mSize++;
                        strtablesize += len + 1;
                        s21 = sym;
                    }
                }
            }
        }
    }
    mSymTable = new Symbol[mSize];
    mStringData = new StringTable(strtablesize);
    mStrTable = new const char *[mSize];
    mUploadedFlags = new bool[mSize];
    if (chunks) {
        int idx = 0;
        auto *origChunks = chunks;
        s21 = Symbol();
        for (int i = 0; i < numElements; i++) {
            Symbol sym = chunks[i].sym.LiteralSym();
            if (sym != s21) {
                mUploadedFlags[idx] = false;
                mSymTable[idx] = sym;
                mStrTable[idx] = mStringData->Add(chunks[i].str.LiteralStr());
                idx++;
                s21 = sym;
            } else {
                MILO_LOG("Locale symbol '%s' redefined\n", s21.Str());
            }
        }
        delete[] origChunks;
    }
    if (localeCfg && localeCfg->Size() > 1) {
        mFile = localeCfg->Str(1);
    }
    DataRegisterFunc("set_locale_verbose_notify", DataSetLocaleVerboseNotify);
    DataRegisterFunc("toggle_show_tokens_cheat", DataToggleShowTokensCheat);
}

void Locale::Terminate() {
    delete[] mSymTable;
    mSymTable = nullptr;
    delete[] mStrTable;
    mStrTable = nullptr;
    delete[] mUploadedFlags;
    mUploadedFlags = nullptr;
    RELEASE(mStringData);
    mSize = 0;
    mFile = Symbol();
    mNumFilesLoaded = 0;
}

void Locale::SetMagnuStrings(DataArray *da) {
    if (mMagnuStrings) {
        mMagnuStrings->Release();
        mMagnuStrings = nullptr;
    }
    mMagnuStrings = da;
}

const char *Locale::Localize(Symbol sym, bool fail) const {
    if (sym.Str() == gNullStr) {
        return "";
    }
    MILO_ASSERT(mSymTable, 0x1d8);

    static Symbol eng("eng");

    if (mMagnuStrings) {
        if (SystemLanguage() == eng) {
            DataArray *arr = mMagnuStrings->FindArray(sym, false);
            if (arr) {
                return arr->Node(1).Str(arr);
            }
        }
    }

    int index;
    if (FindDataIndex(sym, index, fail)) {
        return mStrTable[index];
    }

    if (UsingCD()) {
        SendDebugDataPoint("debug/locale/token", "token", sym, "success", false);
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
