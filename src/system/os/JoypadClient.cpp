#include "os/JoypadClient.h"
#include "JoypadClient.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "os/PlatformMgr.h"
#include "os/User.h"
#include "os/UserMgr.h"
#include "stl/_vector.h"

float gDefaultHoldMs = -1.0f;
float gDefaultRepeatMs = -1.0f;

namespace {
    bool gInited = false;

    JoypadButton LeftStickToDpad(JoypadButton btn) {
        JoypadButton ret;
        switch (btn) {
        case kPad_LStickUp:
            return kPad_DUp;
        case kPad_LStickDown:
            return kPad_DDown;
        case kPad_LStickLeft:
            return kPad_DLeft;
        case kPad_LStickRight:
            return kPad_DRight;
        default:
            MILO_FAIL("illegal button");
            return kPad_NumButtons;
        }
    }
}

JoypadRepeat::JoypadRepeat()
    : mLastBtn(kPad_NumButtons), mLastAction(kAction_None), mLastPad(-1) {}

void JoypadRepeat::Start(JoypadButton btn, JoypadAction act, int pad){
    mHoldTimer.Reset();
    mRepeatTimer.Reset();
    mHoldTimer.Start();
    mLastBtn = btn;
    mLastAction = act;
    mLastPad = pad;
}

void JoypadRepeat::SendRepeat(Hmx::Object *o, int i){
    LocalUser *user = TheUserMgr ? TheUserMgr->GetLocalUserFromPadNum(i) : nullptr;
    o->Handle(ButtonDownMsg(user, mLastBtn, mLastAction, mLastPad), false);
}

void JoypadRepeat::Poll(float f1, float f2, Hmx::Object *o, int i4){
    mHoldTimer.Pause();
    mRepeatTimer.Pause();
    float holdMs = mHoldTimer.Ms();
    float repeatMs = mRepeatTimer.Ms();
    mHoldTimer.Resume();
    mRepeatTimer.Resume();
    if (holdMs >= f1) {
        SendRepeat(o, i4);
        mHoldTimer.Reset();
        mRepeatTimer.Start();
    } else if (repeatMs >= f2) {
        SendRepeat(o, i4);
        mRepeatTimer.Reset();
        mRepeatTimer.Start();
    }
}

JoypadClient::JoypadClient(Hmx::Object *sink)
    : mUser(), mSink(sink), mBtnMask(0), mHoldMs(-1.0f), mRepeatMs(-1.0f),
      mVirtualDpad(0), mFilterAllButStart(0) {
    MILO_ASSERT(sink, 0x88);
    Init();
}

JoypadClient::~JoypadClient(){
    //JoypadUnsubscribe();
}

void JoypadClient::SetVirtualDpad(bool b){
    mVirtualDpad = b;
}

int JoypadClient::OnMsg(ButtonDownMsg const &msg){
    return 0;
}

int JoypadClient::OnMsg(ButtonUpMsg const &msg){
    return 0;
}

void JoypadClient::Poll(){
    for (int i = 0; i < 4; i++) {
        if(ThePlatformMgr.GuideShowing()){
            mRepeats[i].mHoldTimer.Reset();
            mRepeats[i].mRepeatTimer.Reset();
        }
        else{
            mRepeats[i].Poll(mHoldMs, mRepeatMs, mSink, i);
        }
    }
    
}

void JoypadClient::Init(){

}

void JoypadClientPoll(){
}
