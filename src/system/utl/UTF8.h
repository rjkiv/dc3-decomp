#pragma once
#include "utl/Str.h"

unsigned short WToLower(unsigned short);
unsigned short WToUpper(unsigned short);
int WStrniCmp(const unsigned short *str1, const unsigned short *str2, int count);
unsigned int DecodeUTF8(unsigned short &, const char *);
unsigned int EncodeUTF8(String &out, unsigned int in);
void UTF8toASCIIs(char *out, int len, const char *in, char sub);
void ASCIItoUTF8(char *out, int len, const char *in);
unsigned int UTF8StrLen(const char *str);
const char *UTF8strchr(const char *, unsigned short);
// void UTF8ToLower(unsigned short, char *);
// void UTF8ToUpper(unsigned short, char *);
void UTF8FilterString(char *out, int len, const char *in, const char *allowed, char sub);
void UTF8toWChar_t(wchar_t *wc, const char *c);

void ASCIItoWideVector(std::vector<unsigned short> &wideVec, const char *asciiStr);
unsigned int WideVectorToUTF8(const std::vector<unsigned short> &wideVec, String &str);
void UTF8FilterKeyboardString(char *, int, const char *); // defined in os/PlatformMgr.cpp
                                                          // for some reason
String WideVectorToASCII(const std::vector<unsigned short> &wideVec);
const char *WideCharToChar(const unsigned short *wideStr);
void UTF8RemoveSpaces(char *out, int len, const char *in);
void UTF8toUTF16(unsigned short *us, const char *c);

void UTF8toWideVector(std::vector<unsigned short> &, const char *);
const unsigned short *CharToWideChar(const char *);
