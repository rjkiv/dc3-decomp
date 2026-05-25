#pragma once

// there's a whole slew of ifdefs in here for different project configs like stlport
// but i couldn't be bothered to tinker with that so i just hardcoded these lol

#ifdef __cplusplus
#define RADDEFFUNC extern "C"
#define RADDEFSTART extern "C" {
#define RADDEFEND }
#define RADDEFINEDATA extern "C"
#define RADDECLAREDATA extern "C"
#define RADDEFAULT(val) = val
#else
#define RADDEFFUNC
#define RADDEFSTART
#define RADDEFEND
#define RADDEFINEDATA
#define RADDECLAREDATA extern
#define RADDEFAULT(val)
#endif

#define RADLINK __cdecl
#define RADEXPLINK __cdecl
#define RADEXPFUNC RADDEFFUNC
#define PTR4
#define S8 signed char
#define U8 unsigned char
#define U16 unsigned short
#define S16 signed short
#define U32 unsigned int
#define S32 signed int
#define UINTa unsigned long
#define F32 float
#define U64 unsigned long long
