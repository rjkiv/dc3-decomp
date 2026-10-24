#include "os/HolmesKeyboard.h"
#include "obj/Data.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "os/Keyboard.h"
#include "os/PlatformMgr.h"
#include "utl/BinStream.h"
#include "utl/MemStream.h"

HolmesInput::HolmesInput(CWnd *cwnd)
    : mJoypadBuffer(new MemStream(true)), mKeyboardBuffer(new MemStream(true)),
      mOwner(cwnd) {}

HolmesInput::~HolmesInput() {
    RELEASE(mKeyboardBuffer);
    RELEASE(mJoypadBuffer);
}

unsigned int HolmesInput::SendJoypadMessages() {
    static DataNode &n = DataVariable("fake_controllers");
    unsigned int ret = 0;
    mJoypadBuffer->Seek(0, BinStream::kSeekBegin);
    while (mJoypadBuffer->Eof() == NotEof) {
        n = 1;
        bool curScreenSaver = ThePlatformMgr.ScreenSaver();
        ThePlatformMgr.SetScreenSaver(false);
        ThePlatformMgr.SetScreenSaver(curScreenSaver);
        int up;
        JoypadButton button;
        JoypadAction action;
        *mJoypadBuffer >> up;
        *mJoypadBuffer >> (int &)button;
        *mJoypadBuffer >> (int &)action;
        JoypadData *jData = JoypadGetPadData(0);
        if (up == 0) {
            ret |= 1 << button;
            ButtonDownMsg msg(jData->mUser, button, action, 0);
            JoypadPushThroughMsg(msg);
        } else {
            ButtonUpMsg msg(jData->mUser, button, action, 0);
            JoypadPushThroughMsg(msg);
        }
    }
    mJoypadBuffer->Compact();
    return ret;
}

void HolmesInput::SendKeyboardMessages() {
    mKeyboardBuffer->Seek(0, BinStream::kSeekBegin);
    while (mKeyboardBuffer->Eof() == NotEof) {
        bool curScreenSaver = ThePlatformMgr.ScreenSaver();
        ThePlatformMgr.SetScreenSaver(false);
        ThePlatformMgr.SetScreenSaver(curScreenSaver);
        int key;
        bool shift, ctrl, alt;
        *mKeyboardBuffer >> key;
        *mKeyboardBuffer >> shift;
        *mKeyboardBuffer >> ctrl;
        *mKeyboardBuffer >> alt;
        KeyboardSendMsg(key, shift, ctrl, alt);
    }
    mKeyboardBuffer->Compact();
}

void HolmesInput::LoadJoypad(BinStream &d) {
    mKeyboardBuffer->Seek(0, BinStream::kSeekEnd);
    int i20;
    d >> i20;
    if (i20 > 0) {
        mJoypadBuffer->WriteStream(d, i20);
    }
}

void HolmesInput::LoadKeyboard(BinStream &d) {
    mKeyboardBuffer->Seek(0, BinStream::kSeekEnd);
    int i20;
    d >> i20;
    if (i20 > 0) {
        mKeyboardBuffer->WriteStream(d, i20);
    }
}
