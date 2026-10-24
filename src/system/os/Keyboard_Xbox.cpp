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
            // maps to HMX's KB_F1 - KB_F12
            ret = vk + 0x121;
        }
        switch (vk) {
        case VK_BACK:
            ret = KB_BACKSPACE;
            break;
        case VK_TAB:
            ret = KB_TAB;
            break;
        case VK_RETURN:
            ret = KB_ENTER;
            break;
        case VK_PAUSE:
            ret = KB_PAUSE;
            break;
        case VK_CAPITAL:
            ret = KB_CAP_LOCK;
            break;
        case VK_ESCAPE:
            ret = KB_ESCAPE;
            break;
        case VK_PRIOR:
            ret = KB_PAGE_UP;
            break;
        case VK_NEXT:
            ret = KB_PAGE_DOWN;
            break;
        case VK_END:
            ret = KB_END;
            break;
        case VK_HOME:
            ret = KB_HOME;
            break;
        case VK_LEFT:
            ret = KB_LEFT;
            break;
        case VK_UP:
            ret = KB_UP;
            break;
        case VK_RIGHT:
            ret = KB_RIGHT;
            break;
        case VK_DOWN:
            ret = KB_DOWN;
            break;
        case VK_PRINT:
            ret = KB_PRINT;
            break;
        case VK_INSERT:
            ret = KB_INSERT;
            break;
        case VK_DELETE:
            ret = KB_DELETE;
            break;
        case VK_NUMLOCK:
            ret = KB_NUM_LOCK;
            break;
        case VK_SCROLL:
            ret = KB_SCROLL_LOCK;
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
