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

void JoypadInitXboxPCDeadzone(DataArray *arr) {
    arr->FindData("deadzone", gXboxDeadzone);
    gXboxDeadzone /= 256.0f;
}

void TranslateStick(char *keys, short s, bool param_a, bool param_b) {
    float var1 = (s + 0.5f) * 0.000030518044f; // this should be / 32768

    if (param_b) {
        if (var1 > gXboxDeadzone) {
            var1 = (var1 - gXboxDeadzone) / (1 - gXboxDeadzone);
        } else if (var1 < -gXboxDeadzone) {
            var1 = (var1 + gXboxDeadzone) / (1 - gXboxDeadzone);
        } else {
            var1 = 0;
        }
    }
    char c = (var1 * 127);
    *keys = c;

    if (param_a) {
        *keys = -c;
    }
}

void TranslateButtons(unsigned int *buttons, unsigned short s) {
    static int var2[16] = { 0xC, 0xE, 0xF, 0xD, 0xB, 8, 9, 0xA, 2, 3, 0, 0, 6, 5, 7, 4 };
    *buttons = 0;

    for (int i = 0; i < 16; i++) {
        if (s & 1 << i) {
            *buttons = 1 << var2[i] | *buttons;
        }
    }
}

bool JoypadGetCachedXInputCaps(int pad, XINPUT_CAPABILITIES *caps, bool b3) {
    if (gCapsValid[pad] && !b3) {
        *caps = gCaps[pad];
    } else {
        CritSecTracker tracker(&gCritSection);
        if (XInputGetCapabilities(pad, 0, caps) == ERROR_SUCCESS) {
            gCaps[pad] = *caps;
            gCapsValid[pad] = true;
        } else
            return false;
    }
    return true;
}

void JoypadResetXboxPC(int pad) {
    ResetAllUsersPads();
    if (TheUserMgr && TheUserMgr->GetBool()) {
        std::vector<LocalUser *> users;
        TheUserMgr->GetLocalUsers(users);
        for (int i = 0; i < pad; i++) {
            if (i >= users.size())
                break;
            AssociateUserAndPad(users[i], i);
        }
    }
}

JoypadType ReadSingleXinputJoypad(
    int i1,
    int i2,
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
    GetXinputSinceLastFrame(i2, &state, iButtons);
    if (state.dwPacketNumber == -1) {
        return kJoypadNone;
    }

    bool i7 = 0;
    XINPUT_CAPABILITIES caps;
    if (JoypadGetCachedXInputCaps(i2, &caps, false)) {
        switch (caps.SubType) {
        case XINPUT_DEVSUBTYPE_GUITAR:
        case XINPUT_DEVSUBTYPE_GUITAR_BASS:
            ret = SetupHXGuitar(i1, caps);
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
            ret = SetupHXDrums(i1, caps);
            break;
        case 9:
            ret = kJoypadXboxStageKit;
            break;
        case 15:
            ret = SetupHXKeytar(i1, caps);
            break;
        case 25:
            ret = SetupHXRealGuitar(i1, caps);
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
        bool sustain = TheKeyboard ? TheKeyboard->GetSustain(i1) : false;
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
