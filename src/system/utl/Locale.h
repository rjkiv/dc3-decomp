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

class LocaleChunkSort {
public:
    struct OrderedLocaleChunk {
        OrderedLocaleChunk() : node1(0), node2(0), node3(0) {}
        DataNode node1;
        DataNode node2;
        DataNode node3;

        MEM_ARRAY_OVERLOAD(OrderedLocaleChunk, 0x1d)
    };

    static void Sort(OrderedLocaleChunk *, int);
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

const char *Localize(Symbol token, bool *success, Locale &locale);
const char *LocalizeSeparatedInt(int num, Locale &locale);
const char *LocalizeFloat(const char *fmt, float num);
void SyncReloadLocale();
