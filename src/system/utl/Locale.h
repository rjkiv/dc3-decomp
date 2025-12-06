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
    int unk10;
    Symbol mFile; // 0x14
    int mNumFilesLoaded; // 0x18
    bool *mUploadedFlags; // 0x1c
    DataArray *mMagnuStrings; // 0x20
public:
    Locale() {}
    // : mSize(0), mSymTable(0), mStrTable(0), mStringData(0), mUploadedFlags(0),
    //   mFile(), mNumFilesLoaded(0), mMagnuStrings(0) {}
    ~Locale();

    void Init();
    void Terminate();

    // static const char *sIgnoreMissingText;

    void SetMagnuStrings(DataArray *);
    // bool FindDataIndex(Symbol, int &, bool) const;
    // const char *Localize(Symbol, bool) const;

    static void SetLocaleVerboseNotify(bool set) { Locale::sVerboseNotify = set; }

protected:
    static bool sVerboseNotify;
};

extern Locale TheLocale;

inline

    const char *
    Localize(Symbol, bool *, Locale &);
const char *LocalizeSeparatedInt(int, Locale &);
const char *LocalizeFloat(const char *, float);
void SyncReloadLocale();
