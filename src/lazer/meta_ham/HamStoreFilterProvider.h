#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class HamStoreFilter {
public:
    HamStoreFilter(Symbol s);
    HamStoreFilter(DataArray const *);

    const Symbol &GetName() const { return mName; }
    const char *GetText() const { return mText.c_str(); }
    const std::vector<Symbol> &GetSorts() const { return mSorts; }

private:
    Symbol mName; // 0x0
    String mText; // 0x4
    std::vector<Symbol> mSorts; // 0xc
};

class HamStoreFilterProvider : public UIListProvider, public Hmx::Object {
public:
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const;

    HamStoreFilterProvider(std::vector<HamStoreFilter *> *);

private:
    std::vector<HamStoreFilter *> *mFilters; // 0x30
};
