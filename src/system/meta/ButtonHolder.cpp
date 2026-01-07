#include "meta/ButtonHolder.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "os/UserMgr.h"

ProcessedButtonDownMsg::ProcessedButtonDownMsg(
    LocalUser *user, JoypadButton butt, JoypadAction act, int i, bool b
)
    : Message(Type(), user, butt, act, i, b) {}

LocalUser *ProcessedButtonDownMsg::GetUser() const { return mData->Obj<LocalUser>(2); }

ButtonHolder::ButtonHolder(Hmx::Object *callback, UserMgr *mgr) {
    mCallback = callback;
    MILO_ASSERT(mCallback, 0x40);
    if (mgr)
        mUserMgr = mgr;
    else
        mUserMgr = TheUserMgr;
}

BEGIN_HANDLERS(ButtonHolder)
    HANDLE(set_hold_actions, OnSetHoldActions)
    HANDLE_MESSAGE(ButtonDownMsg)
END_HANDLERS

void ButtonHolder::Poll() {
    static Symbol on_button_held("on_button_held");
    std::vector<ActionRec> recs = mActionRecs;
    for (int i = 0; i < 4; i++) {
        if (JoypadIsConnectedPadNum(i)) {
            JoypadData *curPadData = JoypadGetPadData(i);
            for (std::vector<ActionRec>::iterator it = recs.begin();
                 curPadData && it != recs.end();
                 ++it) {
                static ProcessedButtonDownMsg msg(
                    nullptr, kPad_L2, kAction_None, 0, false
                );
                PressRec &pressRec = it->GetPressRec(i);
                if (curPadData->IsButtonInMask(pressRec.iRawButton)) {
                    if (pressRec.fPressTime > 0
                        && TheTaskMgr.UISeconds() - pressRec.fPressTime
                            >= it->mHoldTime) {
                        msg[0] = pressRec.iUser;
                        msg[1] = pressRec.iRawButton;
                        msg[2] = pressRec.iAction;
                        msg[3] = pressRec.iPadNum;
                        msg[4] = 1;
                        mCallback->Handle(msg, true);
                        pressRec.fPressTime = -TheTaskMgr.UISeconds();
                        goto out;
                    }
                } else {
                    if (pressRec.fPressTime > 0) {
                        msg[0] = pressRec.iUser;
                        msg[1] = pressRec.iRawButton;
                        msg[2] = pressRec.iAction;
                        msg[3] = pressRec.iPadNum;
                        msg[4] = 0;
                        mCallback->Handle(msg, true);
                        pressRec.fPressTime = 0;
                        goto out;
                    }
                    if (pressRec.fPressTime < 0)
                        pressRec.fPressTime = 0;
                }
            }
        }
    }
out:
    for (int i = 0; i < mActionRecs.size(); i++) {
        JoypadAction a = mActionRecs[i].mAction;
        auto it = std::find(recs.begin(), recs.end(), a);
        if (it != recs.end()) {
            mActionRecs[i].mPresses = it->mPresses;
        }
    }
}

void ButtonHolder::ClearHeldButtons() {
    std::vector<ActionRec> recs;
    for (int i = 0; i < mActionRecs.size(); i++) {
        ActionRec rec(mActionRecs[i].mAction, mActionRecs[i].mHoldTime, mUserMgr);
        recs.push_back(rec);
    }
    SetHoldActions(recs);
}

void ButtonHolder::SetHoldActions(std::vector<ActionRec> &recs) {
    mActionRecs.clear();
    mActionRecs = recs;
}

DataNode ButtonHolder::OnSetHoldActions(DataArray *da) {
    std::vector<ActionRec> recs;
    DataArray *arr = da->Array(2);
    for (int i = 0; i < arr->Size(); i++) {
        DataArray *innerArr = arr->Array(i);
        float innerFloat = innerArr->Float(1);
        if (innerFloat > 0) {
            ActionRec rec((JoypadAction)innerArr->Int(0), innerFloat, mUserMgr);
            recs.push_back(rec);
        }
    }
    SetHoldActions(recs);
    return 1;
}

DataNode ButtonHolder::OnMsg(const ButtonDownMsg &msg) {
    auto it = std::find(mActionRecs.begin(), mActionRecs.end(), msg.GetAction());
    if (it != mActionRecs.end()) {
        PressRec &pressRec = it->GetPressRec(msg.GetUser()->GetPadNum());
        pressRec.iRawButton = msg.GetButton();
        pressRec.iAction = msg.GetAction();
        pressRec.fPressTime = TheTaskMgr.UISeconds();
        pressRec.iPadNum = msg.GetPadNum();
        return 1;
    } else
        return DATA_UNHANDLED;
}

ActionRec::ActionRec(JoypadAction act, float f, UserMgr *umgr)
    : mAction(act), mHoldTime(f) {
    std::vector<LocalUser *> uservec;
    umgr->GetLocalUsers(uservec);
    for (int i = 0; i < uservec.size(); i++) {
        mPresses.push_back(PressRec());
        mPresses[i].iUser = uservec[i];
        mPresses[i].iRawButton = kPad_L2;
        mPresses[i].iAction = kAction_None;
        mPresses[i].fPressTime = 0;
        mPresses[i].iPadNum = 0;
    }
}

PressRec &ActionRec::GetPressRec(int padnum) {
    for (int i = 0; i < mPresses.size(); i++) {
        if (mPresses[i].iUser->GetPadNum() == padnum)
            return mPresses[i];
    }
    MILO_FAIL("No PressRec exists for this padnum");
    return mPresses[0];
}
