#pragma once
#include "obj/Object.h"

void KeyboardInitCommon();
void KeyboardTerminateCommon();

void KeyboardInit();
void KeyboardTerminate();
void KeyboardPoll();

void KeyboardSubscribe(Hmx::Object *);
void KeyboardUnsubscribe(Hmx::Object *);
Hmx::Object *KeyboardOverride(Hmx::Object *);
void KeyboardSendMsg(int key, bool shift, bool ctrl, bool alt);

#include "obj/Msg.h"

DECLARE_MESSAGE(KeyboardKeyMsg, "key")
KeyboardKeyMsg(int key, bool shift, bool ctrl, bool alt)
    : Message(Type(), key, shift, ctrl, alt) {}
int GetKey() const { return mData->Int(2); }
bool GetShift() const { return mData->Int(3); }
bool GetCtrl() const { return mData->Int(4); }
bool GetAlt() const { return mData->Int(5); }
END_MESSAGE

DECLARE_MESSAGE(KeyboardKeyReleaseMsg, "key_release")
END_MESSAGE
