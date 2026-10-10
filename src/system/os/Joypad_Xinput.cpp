#include "os/Joypad_Xinput.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Joypad.h"
#include "os/Joypad_Xbox.h"
#include "os/UsbMidiKeyboard.h"
#include "os/UserMgr.h"
#include "xdk/XAPILIB.h"

namespace {
    XINPUT_CAPABILITIES gCaps[kNumJoypads];
    static float gXboxDeadzone = 0;
    static unsigned char gTriggerThreshold = 0;
    bool gCapsValid[kNumJoypads];
    CriticalSection gCritSection;
}

void JoypadInitXboxPCDeadzone(DataArray *joypad_config) {
    joypad_config->FindData("deadzone", gXboxDeadzone);
    gXboxDeadzone /= 256.0f;
}

void TranslateStick(char *out, short in, bool flip, bool deadzone) {
    float var1 = (in + 0.5f) * 0.000030518044f; // this should be / 32768

    if (deadzone) {
        if (var1 > gXboxDeadzone) {
            var1 = (var1 - gXboxDeadzone) / (1 - gXboxDeadzone);
        } else if (var1 < -gXboxDeadzone) {
            var1 = (var1 + gXboxDeadzone) / (1 - gXboxDeadzone);
        } else {
            var1 = 0;
        }
    }
    char c = (var1 * 127);
    *out = c;

    if (flip) {
        *out = -c;
    }
}

void TranslateButtons(unsigned int *out, unsigned short digButs) {
    static int var2[16] = { 0xC, 0xE, 0xF, 0xD, 0xB, 8, 9, 0xA, 2, 3, 0, 0, 6, 5, 7, 4 };
    *out = 0;

    for (int i = 0; i < 16; i++) {
        if (digButs & 1 << i) {
            *out = 1 << var2[i] | *out;
        }
    }
}

bool JoypadGetCachedXInputCaps(int xinput_id, XINPUT_CAPABILITIES *pxcaps, bool forceFreshGet) {
    if (gCapsValid[xinput_id] && !forceFreshGet) {
        *pxcaps = gCaps[xinput_id];
    } else {
        CritSecTracker tracker(&gCritSection);
        if (XInputGetCapabilities(xinput_id, 0, pxcaps) == ERROR_SUCCESS) {
            gCaps[xinput_id] = *pxcaps;
            gCapsValid[xinput_id] = true;
        } else
            return false;
    }
    return true;
}

void JoypadResetXboxPC(int numJoypads) {
    ResetAllUsersPads();
    if (TheUserMgr && TheUserMgr->GetBool()) {
        std::vector<LocalUser *> users;
        TheUserMgr->GetLocalUsers(users);
        for (int i = 0; i < numJoypads; i++) {
            if (i >= users.size())
                break;
            AssociateUserAndPad(users[i], i);
        }
    }
}

JoypadType ReadSingleXinputJoypad(
    int iPadNum,
    int xinput_id,
    unsigned int *iButtons,
    char *iLeftStickX,
    char *iLeftStickY,
    char *iRightStickX,
    char *iRightStickY,
    char *iLeftTrigger,
    char *iRightTrigger,
    float *const f10,
    float *const f11,
    unsigned char *const uc12
) {
    JoypadType ret = kJoypadAnalog;
    XINPUT_STATE state;
    GetXinputSinceLastFrame(xinput_id, &state, iButtons);
    if (state.dwPacketNumber == -1) {
        return kJoypadNone;
    }

    bool i7 = 0;
    XINPUT_CAPABILITIES caps;
    if (JoypadGetCachedXInputCaps(xinput_id, &caps, false)) {
        switch (caps.SubType) {
        case XINPUT_DEVSUBTYPE_GUITAR:
        case XINPUT_DEVSUBTYPE_GUITAR_BASS:
            ret = SetupHXGuitar(iPadNum, caps);
            if (ret == kJoypadNone) {
                return kJoypadNone;
            }
            if (ret != kJoypadXboxMidiBoxKeyboard) {
                i7 = 1;
            }
            break;
        case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
            ret = kJoypadXboxRoGuitar;
            i7 = 1;
            break;
        case XINPUT_DEVSUBTYPE_DRUM_KIT:
            ret = SetupHXDrums(iPadNum, caps);
            break;
        case 9:
            ret = kJoypadXboxStageKit;
            break;
        case 15:
            ret = SetupHXKeytar(iPadNum, caps);
            break;
        case 25:
            ret = SetupHXRealGuitar(iPadNum, caps);
            break;
        default:
            break;
        }
    }
    bool b3 = i7 == 0;
    TranslateStick(iLeftStickX, state.Gamepad.sThumbLX, false, b3);

    if (ret == kJoypadXboxDrums && state.Gamepad.sThumbLY > 0
        && state.Gamepad.sThumbLY < 256) {
        float clamped = Clamp(27.0f, 122.0f, (float)state.Gamepad.sThumbLY);
        short sLY = (clamped - 27.0f) * 0.010526316f * -26539.0f;
        TranslateStick(iLeftStickY, -0x8000 - sLY, true, false);
    } else {
        TranslateStick(iLeftStickY, state.Gamepad.sThumbLY, true, b3);
    }

    if (ret == kJoypadXboxDrums && state.Gamepad.sThumbRX > 0
        && state.Gamepad.sThumbRX < 256) {
        if (state.Gamepad.sThumbRX > 0 && state.Gamepad.sThumbRX < 256) {
            float clamped = Clamp(27.0f, 122.0f, (float)state.Gamepad.sThumbRX);
            short sRX = (clamped - 27.0f) * 0.010526316f * -26539.0f;
            TranslateStick(iRightStickX, -0x8000 - sRX, true, false);
        } else if (state.Gamepad.sThumbRX > 256) {
            TranslateStick(iRightStickX, state.Gamepad.sThumbRX, true, b3);
        } else {
            TranslateStick(iRightStickX, state.Gamepad.sThumbRX, false, b3);
        }
    } else {
        TranslateStick(iRightStickX, state.Gamepad.sThumbRX, false, b3);
    }
    TranslateStick(iRightStickY, state.Gamepad.sThumbRY, true, i7 == 0 && ret != 8);

    if (ret == kJoypadXboxMidiBoxKeyboard || ret == kJoypadXboxKeytar) {
        bool sustain = TheKeyboard ? TheKeyboard->GetSustain(iPadNum) : false;
        if (sustain) {
            *iButtons |= 4;
        } else {
            *iButtons &= ~4;
        }
    }

    if (ret == kJoypadAnalog) {
        if (state.Gamepad.bLeftTrigger > gTriggerThreshold) {
            *iButtons |= 1;
        } else {
            *iButtons &= ~1;
        }
        if (state.Gamepad.bRightTrigger > gTriggerThreshold) {
            *iButtons |= 2;
        } else {
            *iButtons &= ~2;
        }
    }
    *iLeftTrigger = state.Gamepad.bLeftTrigger & 0x7F;
    *iRightTrigger = state.Gamepad.bRightTrigger & 0x7F;
    return ret;
}
