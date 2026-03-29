#include "os/HolmesKeyboard.h"
#include "obj/Data.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "os/Keyboard.h"
#include "os/PlatformMgr.h"
#include "utl/BinStream.h"
#include "utl/MemStream.h"

HolmesInput::HolmesInput(CWnd *cwnd)
    : mJoypadStream(new MemStream(true)), mKeyboardStream(new MemStream(true)),
      mCWnd(cwnd) {}

HolmesInput::~HolmesInput() {
    RELEASE(mKeyboardStream);
    RELEASE(mJoypadStream);
}

unsigned int HolmesInput::SendJoypadMessages() {
    static DataNode &n = DataVariable("fake_controllers");
    unsigned int ret = 0;
    mJoypadStream->Seek(0, BinStream::kSeekBegin);
    while (mJoypadStream->Eof() == NotEof) {
        n = 1;
        bool curScreenSaver = ThePlatformMgr.ScreenSaver();
        ThePlatformMgr.SetScreenSaver(false);
        ThePlatformMgr.SetScreenSaver(curScreenSaver);
        int up;
        JoypadButton button;
        JoypadAction action;
        *mJoypadStream >> up;
        *mJoypadStream >> (int &)button;
        *mJoypadStream >> (int &)action;
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
    mJoypadStream->Compact();
    return ret;
}

void HolmesInput::SendKeyboardMessages() {
    mKeyboardStream->Seek(0, BinStream::kSeekBegin);
    while (mKeyboardStream->Eof() == NotEof) {
        bool curScreenSaver = ThePlatformMgr.ScreenSaver();
        ThePlatformMgr.SetScreenSaver(false);
        ThePlatformMgr.SetScreenSaver(curScreenSaver);
        int key;
        bool shift, ctrl, alt;
        *mKeyboardStream >> key;
        *mKeyboardStream >> shift;
        *mKeyboardStream >> ctrl;
        *mKeyboardStream >> alt;
        KeyboardSendMsg(key, shift, ctrl, alt);
    }
    mKeyboardStream->Compact();
}

void HolmesInput::LoadJoypad(BinStream &bs) {
    mKeyboardStream->Seek(0, BinStream::kSeekEnd);
    int i20;
    bs >> i20;
    if (i20 > 0) {
        mJoypadStream->WriteStream(bs, i20);
    }
}

void HolmesInput::LoadKeyboard(BinStream &bs) {
    mKeyboardStream->Seek(0, BinStream::kSeekEnd);
    int i20;
    bs >> i20;
    if (i20 > 0) {
        mKeyboardStream->WriteStream(bs, i20);
    }
}
