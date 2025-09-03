#pragma once
#include "os/Joypad.h"

void JoypadInitXboxPCDeadzone(class DataArray *);
void TranslateStick(char *, short, bool, bool);
void TranslateButtons(unsigned int *, unsigned short);
bool JoypadGetCachedXInputCaps(int, struct _XINPUT_CAPABILITIES *, bool);  

JoypadType ReadSingleXinputJoypad(int, int, unsigned int *, char *, char *, 
char*, char*, char* ,char*, char *, float *, float *, unsigned char *);

void JoypadResetXboxPC(int);
