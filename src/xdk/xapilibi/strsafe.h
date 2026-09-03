#pragma once

#include "win_types.h"
#include <stddef.h>

HRESULT StringCbPrintfA(LPSTR pszDest, size_t cbDest, LPCSTR pszFormat, ...);
