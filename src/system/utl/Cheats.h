#pragma once

#include "obj/Data.h"
#include "os/User.h"
void EnableKeyCheats(bool);
bool GetEnabledKeyCheats();
bool CheatsInitialized();
void CallQuickCheat(DataArray *a, LocalUser *u);
