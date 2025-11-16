#pragma once
#include "utl/Locale.h"

const char *LocalizeOrdinal(int, LocaleGender, LocaleNumber, bool);
const char *LocalizeOrdinal(int, LocaleGender, LocaleNumber, bool, Symbol, Locale &);
