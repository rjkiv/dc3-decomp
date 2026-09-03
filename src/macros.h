#pragma once

#define MILO_DEBUG

#ifdef _MSC_VER
#define CDECL __cdecl
#define STDCALL __stdcall
#define FASTCALL __fastcall

#define SEH_TRY __try
#define SEH_EXCEPT __except
#define SEH_FINALLY __finally

#define ALIGN(x) __declspec(align(x))
#define INTRIN __declspec(intrin_type)
#else
#define CDECL
#define STDCALL
#define FASTCALL

#define SEH_TRY try
#define SEH_EXCEPT catch
#define SEH_FINALLY ;

#define ALIGN(x) alignas(x)
#define INTRIN
#endif

// #define ROTATE_LEFT(x, i) (((x) << (i)) | ((x) >> ((sizeof((x)) * 8) - (i))))
// #define ROTATE_RIGHT(x, i) (((x) >> (i)) | ((x) << ((sizeof((x)) * 8) - (i))))

// this is how HMX had it going off asserts
// clang-format off
#define DIM(x) ( sizeof(x)/sizeof(x[0]) )
// clang-format on

#define null 0

#define ROUNDUP(x, n) ((x) + ((n) - 1) & ~((n) - 1))
#define RELEASE(x) (delete x, x = null)
#define RELEASEARRAY(x) (delete[] (ubyte *)x, x = null)
