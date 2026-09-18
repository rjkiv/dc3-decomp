#include "ui/Utl.h"
#include "os/Joypad.h"
#include "ui/UI.h"

int PageDirection(JoypadAction act) {
    if (act == kAction_PageDown) {
        return 1;
    } else if (act == kAction_PageUp) {
        return -1;
    } else {
        return 0;
    }
}

bool IsNavAction(JoypadAction act) {
    return act == kAction_Up || act == kAction_Down || act == kAction_Left
        || act == kAction_Right;
}

int ScrollDirection(const ButtonDownMsg &msg, bool b1, bool b2, int i) {
    JoypadAction act = msg.GetAction();
    if (!b2) {
        if (TheUI->OverloadHorizontalNav(act, msg.GetButton(), b1)) {
            if (act == kAction_Up) {
                act = kAction_Left;
            } else if (act == kAction_Down) {
                act = kAction_Right;
            }
        }
    }
    JoypadAction topLeft = (b2 ? kAction_Up : kAction_Left);
    JoypadAction a78 = (b2 ? kAction_Right : kAction_Down);
    if (act == topLeft) {
        return -i;
    } else if (act == (b2 ? kAction_Down : kAction_Right)) {
        return i;
    } else {
        if (act == (b2 ? kAction_Left : kAction_Up) && i > 1) {
            return -1;
        } else if (act == a78) {
            if (i > 1) {
                return 1;
            }
        }
        return 0;
    }
}
