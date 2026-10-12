#pragma once

#ifndef _SIZE_T_DEFINED
#ifdef _MSC_VER
typedef unsigned int size_t; // or whatever PPC XDK used
#else
typedef unsigned long size_t;
#endif
#define _SIZE_T_DEFINED
#endif

#if !defined(_WCHAR_T_DEFINED) && defined(_MSC_VER)
typedef unsigned short wchar_t; // or whatever PPC XDK used
#define _WCHAR_T_DEFINED
#endif

// #ifndef _FILE_DEFINED
// typedef struct _iobuf FILE; // or your platform's FILE struct
// #define _FILE_DEFINED
// #endif

#ifndef EOF
#define EOF (-1)
#endif
