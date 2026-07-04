#pragma once
#include "../win_types.h"
#include "minwinbase.h"
#include "wtypesbase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XINPUT_BATTERY_INFORMATION {
    BYTE BatteryType;
    BYTE BatteryLevel;
} XINPUT_BATTERY_INFORMATION, *PXINPUT_BATTERY_INFORMATION;

typedef struct _XINPUT_GAMEPAD {
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_KEYSTROKE {
    WORD VirtualKey;
    WCHAR Unicode;
    WORD Flags;
    BYTE UserIndex;
    BYTE HidCode;
} XINPUT_KEYSTROKE, *PXINPUT_KEYSTROKE;

typedef struct _XINPUT_STATE {
    DWORD dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef struct _XINPUT_VIBRATION {
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
} XINPUT_VIBRATION, *PXINPUT_VIBRATION;

typedef struct _XINPUT_CAPABILITIES {
    BYTE Type;
    BYTE SubType;
    WORD Flags;
    XINPUT_GAMEPAD Gamepad;
    XINPUT_VIBRATION Vibration;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

// source:
// https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-controller-subtypes

#define XINPUT_DEVSUBTYPE_UNKNOWN 0
#define XINPUT_DEVSUBTYPE_GAMEPAD 1
#define XINPUT_DEVSUBTYPE_WHEEL 2
#define XINPUT_DEVSUBTYPE_ARCADE_STICK 3
#define XINPUT_DEVSUBTYPE_FLIGHT_STICK 4
#define XINPUT_DEVSUBTYPE_DANCE_PAD 5
#define XINPUT_DEVSUBTYPE_GUITAR 6
#define XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE 7
#define XINPUT_DEVSUBTYPE_DRUM_KIT 8
#define XINPUT_DEVSUBTYPE_GUITAR_BASS 11
#define XINPUT_DEVSUBTYPE_ARCADE_PAD 19

void XInputEnable(BOOL enable);
DWORD XInputGetAudioDeviceIds(
    DWORD dwUserIndex,
    LPWSTR pRenderDeviceId,
    UINT *pRenderCount,
    LPWSTR pCaptureDeviceId,
    UINT *pCaptureCount
);
DWORD XInputGetBatteryInformation(
    DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION *pBatteryInformation
);
DWORD XInputGetCapabilities(
    DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES *pCapabilities
);
DWORD XInputGetKeystroke(
    DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeystroke
);
DWORD XInputGetKeystrokeEx(
    PDWORD pdwUserIndex, DWORD dwFlags, PXINPUT_KEYSTROKE pKeystroke
);
DWORD XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState);
DWORD XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

#ifdef __cplusplus
}
#endif
