#pragma once
#include "utl/Str.h"
#include "utl/Symbol.h"

Symbol GetStarsToken(int);
String GetSeconds(int);
String GetMinutes(int);
String GetHours(int);
String GetDays(int);
void GetTimeString(int, char *);
char const *FormatTimeHMS(int);
char const *FormatTimeMS(int);
