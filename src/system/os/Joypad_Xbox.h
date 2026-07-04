#pragma once
#include "os/Joypad.h"
#include "xdk/XAPILIB.h"

JoypadType SetupHXKeytar(int, const XINPUT_CAPABILITIES &);
JoypadType SetupHXRealGuitar(int, const XINPUT_CAPABILITIES &);
JoypadType SetupHXGuitar(int, const XINPUT_CAPABILITIES &);
JoypadType SetupHXDrums(int, const XINPUT_CAPABILITIES &);

void GetXinputSinceLastFrame(int pad, XINPUT_STATE *state, unsigned int *ui3);
