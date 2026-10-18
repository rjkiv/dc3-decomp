#pragma once
#include "types.h"

void WordWrap_SetOption(uint option);

/** Determines if a line break is allowed between two characters. Follows kinsoku line
    breaking rules for CJK unicode text. */
bool WordWrap_CanBreakLineAt(wchar_t const *beforeBreak, wchar_t const *afterBreak);
