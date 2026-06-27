#pragma once
#include "utl/Symbol.h"
#include "utl/StringTable.h"
#include "obj/Data.h"

enum LocaleGender {
    LocaleGenderMasculine = 0,
    LocaleGenderFeminine = 1,
};

enum LocaleNumber {
    LocaleSingular = 0,
    LocalePlural = 1,
};

class Locale {
private:
    int mSize; // 0x0
    Symbol *mSymTable; // 0x4
    const char **mStrTable; // 0x8
    StringTable *mStringData; // 0xc
    bool *mUploadedFlags; // 0x10
    Symbol mFile; // 0x14
    int mNumFilesLoaded; // 0x18
    bool mInitialized; // 0x1c - checked in Init
    DataArray *mMagnuStrings; // 0x20
public:
    Locale() {}
    ~Locale() {
        if (mMagnuStrings) {
            mMagnuStrings->Release();
            mMagnuStrings = nullptr;
        }
    }

    void Init();
    void Terminate();

    static const char *sIgnoreMissingText;

    void SetMagnuStrings(DataArray *);
    const char *Localize(Symbol token, bool fail = true) const;

    static void SetLocaleVerboseNotify(bool set) { Locale::sVerboseNotify = set; }
    static bool GetLocaleVerboseNotify() { return sVerboseNotify; }

protected:
    bool FindDataIndex(Symbol sym, int &index, bool fail = true) const;

    static bool sVerboseNotify;
};

extern Locale TheLocale;

const char *Localize(Symbol token, bool *success, Locale &locale);
const char *LocalizeSeparatedInt(int num, Locale &locale);
const char *LocalizeFloat(const char *fmt, float num);
void SyncReloadLocale();
