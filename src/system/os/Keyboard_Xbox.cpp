#include "os/Keyboard.h"
#include "os/HolmesClient.h"
#include "xdk/XAPILIB.h"
#include <cctype>

namespace {
    int TranslateVK(unsigned short vk, bool shifted) {
        int ret = 0;
        if ((vk >= VK_0 && vk <= VK_9) || (vk >= VK_A && vk <= VK_Z)) {
            if (shifted) {
                ret = vk;
            } else {
                ret = tolower(vk);
            }
        }
        if (vk >= VK_F1 && vk <= VK_F12) {
            ret = vk + 0x121;
        }
        switch (vk) {
        case VK_BACK:
            ret = 8;
            break;
        case VK_TAB:
            ret = 9;
            break;
        case VK_RETURN:
            ret = 10;
            break;
        case VK_PAUSE:
            ret = 0x12D;
            break;
        case VK_CAPITAL:
            ret = 0x122;
            break;
        case VK_ESCAPE:
            ret = 0x12E;
            break;
        case VK_PRIOR:
            ret = 0x13A;
            break;
        case VK_NEXT:
            ret = 0x13B;
            break;
        case VK_END:
            ret = 0x139;
            break;
        case VK_HOME:
            ret = 0x138;
            break;
        case VK_LEFT:
            ret = 0x140;
            break;
        case VK_UP:
            ret = 0x142;
            break;
        case VK_RIGHT:
            ret = 0x141;
            break;
        case VK_DOWN:
            ret = 0x143;
            break;
        case VK_PRINT:
            ret = 300;
            break;
        case VK_INSERT:
            ret = 0x136;
            break;
        case VK_DELETE:
            ret = 0x137;
            break;
        case VK_NUMLOCK:
            ret = 0x123;
            break;
        case VK_SCROLL:
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
    if (res != ERROR_EMPTY && res == ERROR_SUCCESS) {
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
