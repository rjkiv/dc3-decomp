#include "os/Keyboard.h"
#include "os/HolmesClient.h"
#include "xdk/XAPILIB.h"
#include <cctype>

namespace {
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    int TranslateVK(unsigned short vk, bool shifted) {
        int ret = 0;
        if ((vk >= 0x30 && vk <= 0x39) || (vk >= 0x41 && vk <= 0x5A)) {
            if (shifted) {
                ret = vk;
            } else {
                ret = tolower(vk);
            }
        }
        if (vk >= 0x70 && vk <= 0x7B) {
            ret = vk + 0x121;
        }
        switch (vk) {
        case 8:
            ret = 8;
            break;
        case 9:
            ret = 9;
            break;
        case 0xD:
            ret = 10;
            break;
        case 0x13:
            ret = 0x12D;
            break;
        case 0x14:
            ret = 0x122;
            break;
        case 0x1B:
            ret = 0x12E;
            break;
        case 0x21:
            ret = 0x13A;
            break;
        case 0x22:
            ret = 0x13B;
            break;
        case 0x23:
            ret = 0x139;
            break;
        case 0x24:
            ret = 0x138;
            break;
        case 0x25:
            ret = 0x140;
            break;
        case 0x26:
            ret = 0x142;
            break;
        case 0x27:
            ret = 0x141;
            break;
        case 0x28:
            ret = 0x143;
            break;
        case 0x2A:
            ret = 300;
            break;
        case 0x2D:
            ret = 0x136;
            break;
        case 0x2E:
            ret = 0x137;
            break;
        case 0x90:
            ret = 0x123;
            break;
        case 0x91:
            ret = 0x124;
            break;
        }
        return ret;
    }
}

void KeyboardInit() { KeyboardInitCommon(); }
void KeyboardTerminate() { KeyboardTerminateCommon(); }

void KeyboardPoll() {
    XINPUT_KEYSTROKE keystroke;
    DWORD res = XInputGetKeystroke(XUSER_INDEX_ANY, 2, &keystroke);
    if (res != 0x10D2 && res == 0) {
        if (!(keystroke.Flags & 2)) {
            WCHAR w[2] = { keystroke.Unicode, 0 };
            char c;
            bool shift = keystroke.Flags & 8;
            bool ctrl = keystroke.Flags & 16;
            bool alt = keystroke.Flags & 32;
            WideCharToMultiByte(0, 0, w, 1, &c, 2, nullptr, nullptr);
            int key;
            if (c != '\0' && c >= ' ' && c <= '~') {
                key = c;
            } else {
                key = TranslateVK(keystroke.VirtualKey, shift);
            }
            if (key != 0) {
                KeyboardSendMsg(key, shift, ctrl, alt);
            }
        }
    }
    HolmesClientPollKeyboard();
}
