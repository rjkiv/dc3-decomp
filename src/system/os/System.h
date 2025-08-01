#pragma once
#include "obj/Data.h"
#include "utl/Symbol.h"

int Hx_snprintf(char *, unsigned int, char const *, ...);

extern const char *gNullStr;

DataArray *SystemConfig();
DataArray *SystemConfig(Symbol);
DataArray *SystemConfig(Symbol, Symbol);
DataArray *SystemConfig(Symbol, Symbol, Symbol);
