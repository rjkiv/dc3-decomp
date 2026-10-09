#pragma once
#include "obj/Data.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

class SuperFormatString : public FormatString {
public:
    SuperFormatString(const char *, const DataArray *, bool, Locale &, Symbol);

    const char *FinalStr();

private:
    bool mTokensOnly;
    bool mNeedsDoublePercentCollapse;
};
