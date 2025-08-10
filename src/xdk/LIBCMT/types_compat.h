#pragma once

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t; // or whatever PPC XDK used
#define _SIZE_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
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
