#pragma once
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"

int PageDirection(JoypadAction);
bool IsNavAction(JoypadAction);
int ScrollDirection(ButtonDownMsg const &, bool, bool, int);
