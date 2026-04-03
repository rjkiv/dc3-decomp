#include "os/Joypad_Xinput.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Joypad.h"
#include "os/UserMgr.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/winerror.h"

namespace {
    XINPUT_CAPABILITIES gCaps[kNumJoypads];
    float gXboxDeadzone;
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
