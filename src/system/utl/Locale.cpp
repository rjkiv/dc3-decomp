#include "utl/Locale.h"
#include "DataPointMgr.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Str.h"
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
