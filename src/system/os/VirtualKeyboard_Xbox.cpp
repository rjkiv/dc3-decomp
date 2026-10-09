#include "os/Debug.h"
#include "os/VirtualKeyboard.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "utl/UTF8.h"
#include "xdk/XAPILIB.h"

namespace {
    bool gCheckOverlappedIoComplete;
    XOVERLAPPED *gXoKeyboard;
    wchar_t *gWstrKeyboard;
    char gCstrKeyboard[512];
    wchar_t *gDefaultText;
    wchar_t *gWindowTitle;
    wchar_t *gDescrptionText;
}

void VirtualKeyboard::PlatformPoll() {
    if (gCheckOverlappedIoComplete && gXoKeyboard && gXoKeyboard->InternalLow != 0x3E5) {
        if (XGetOverlappedExtendedError(gXoKeyboard) == 0) {
            char buf[512];
            int ret = WideCharToMultiByte(
                0xFDE9, 0, gWstrKeyboard, -1, buf, 0x200, nullptr, nullptr
            );
            MILO_ASSERT(ret, 0x35);
            char utf8buf[512];
            UTF8FilterKeyboardString(utf8buf, 0x200, buf);
            RemoveSpaces(buf, 0x200, utf8buf);
            mCallbackMsg = buf;
            mMsgOk = true;
        } else {
            mCallbackMsg = gNullStr;
            mMsgOk = false;
        }
        mCallbackReady = true;
        gCheckOverlappedIoComplete = false;
    }
}

const char *VirtualKeyboard::GetInputString() {
    if (gWstrKeyboard && *gWstrKeyboard) {
        memset(gCstrKeyboard, 0, 512);
        int ret = WideCharToMultiByte(
            0xFDE9, 0, gWstrKeyboard, -1, gCstrKeyboard, 0x200, nullptr, nullptr
        );
        MILO_ASSERT(ret, 0xCC);
        char buf[512];
        UTF8FilterKeyboardString(buf, 0x200, gCstrKeyboard);
        RemoveSpaces(gCstrKeyboard, 0x200, buf);
        return gCstrKeyboard;
    } else {
        return gNullStr;
    }
}

DataNode VirtualKeyboard::ShowKeyboardUI(
    int pad, int textLength, String strWindowTitle, String strDescription, String strDefaultText, int configType
) {
    MILO_ASSERT(!mCallbackReady, 0x62);
    RELEASE(gXoKeyboard);
    RELEASE(gWstrKeyboard);
    wchar_t *newWStr = new wchar_t[textLength];
    XOVERLAPPED *newXo = new XOVERLAPPED();
    MILO_ASSERT(pad != -1, 0x7F);
    UTF8toWChar_t(gDefaultText, strDefaultText.c_str());
    UTF8toWChar_t(gWindowTitle, strWindowTitle.c_str());
    UTF8toWChar_t(gDescrptionText, strDescription.c_str());
    DWORD flags = 0;
    if (configType == 0) {
        flags = 0x20000001;
    } else if (configType == 1) {
        flags = 0x20;
    } else {
        MILO_ASSERT(false, 0x98);
    }
    DWORD res = XShowKeyboardUI(
        pad, flags, gDefaultText, gWindowTitle, gDescrptionText, newWStr, textLength, newXo
    );
    if (res != 0x3E5) {
        delete newWStr;
        delete newXo;
        MILO_NOTIFY("Unable to show Keyboard UI.  XShowKeyboardUI returned %d.\n", res);
        mCallbackMsg = gNullStr;
        mMsgOk = false;
        mCallbackReady = true;
        return 0;
    } else {
        gWstrKeyboard = newWStr;
        gXoKeyboard = newXo;
        gCheckOverlappedIoComplete = true;
        return 0;
    }
}
