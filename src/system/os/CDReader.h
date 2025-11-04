#pragma once
#include "types.h"

int CDRead(int, int, int, void *);
bool CDReadExternal(void *&, int, u64);
bool CDReadDone();
