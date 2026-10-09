#include "os/VirtualKeyboard.h"
#include "obj/Dir.h"
#include "os/User.h"
#include "utl/Symbol.h"

VirtualKeyboard TheVirtualKeyboard;

VirtualKeyboard::VirtualKeyboard()
    : mPobjKeyboardCallback(nullptr), mCallbackReady(false), mMsgOk(false) {}

VirtualKeyboard::~VirtualKeyboard() {}

void VirtualKeyboard::Init() { SetName("virtual_keyboard", ObjectDir::Main()); }

VirtualKeyboardResultMsg::VirtualKeyboardResultMsg(bool ok, const char *text)
    : Message(Type(), ok, text ? text : gNullStr) {}

void VirtualKeyboard::Poll() {
    PlatformPoll();
    if (mCallbackReady) {
        VirtualKeyboardResultMsg msg(mMsgOk, mCallbackMsg.c_str());
        if (mPobjKeyboardCallback) {
            mPobjKeyboardCallback->Handle(msg, true);
        }
        mPobjKeyboardCallback = nullptr;
        mCallbackMsg = gNullStr;
        mMsgOk = false;
        mCallbackReady = false;
    }
}

DataNode VirtualKeyboard::OnShowKeyboardUI(const DataArray *da) {
    int i2 = da->Int(2);
    int i3 = da->Int(3);
    class String s4(da->Str(4));
    class String s5(da->Str(5));
    class String s6(da->Str(6));
    mPobjKeyboardCallback = da->GetObj(7);
    int i8 = 0;
    if (da->Size() >= 9)
        i8 = da->Int(8);
    return ShowKeyboardUI(i2, i3, s4, s5, s6, i8);
}

void VirtualKeyboard::ClearKeyboardCallback() { mPobjKeyboardCallback = nullptr; }

BEGIN_HANDLERS(VirtualKeyboard)
    HANDLE(show_keyboard, OnShowKeyboardUI)
    HANDLE_ACTION(clear_callback, ClearKeyboardCallback())
    HANDLE_EXPR(get_input_string, GetInputString())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
