#pragma once

#define MILO_DEBUG

#ifdef _MSC_VER
#define CDECL __cdecl
#define STDCALL __stdcall
#define FASTCALL __fastcall

#define SEH_TRY __try
#define SEH_EXCEPT __except
#define SEH_FINALLY __finally
#else
#define CDECL
#define STDCALL
#define FASTCALL

#define SEH_TRY try
#define SEH_EXCEPT catch
#define SEH_FINALLY ;
#endif
