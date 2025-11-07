#include "ui/Utl.h"
#include "os/Joypad.h"
#include "ui/UI.h"

int PageDirection(JoypadAction act) {
    if (act == kAction_PageDown)
        return 1;

    return act != kAction_PageUp;
}

bool IsNavAction(JoypadAction act) {
    return act == kAction_Up || act == kAction_Down || act == kAction_Left
        || act == kAction_Right;
}

int ScrollDirection(const ButtonDownMsg &msg, bool b1, bool b2, int i) {
    int msgInt = msg.mData->Int(4);
    if (!b2) {
        int msgInt2 = msg.mData->Int(3);
    }
    return i;
}
