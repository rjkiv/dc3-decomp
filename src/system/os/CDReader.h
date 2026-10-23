#pragma once
#include "types.h"

int CDRead(int arkfileNum, int sector, int numSectors, void *buf);
bool CDReadExternal(void *&out_handle, int arkfileNum, u64 startByte);
bool CDReadDone();
int CDGetError();
