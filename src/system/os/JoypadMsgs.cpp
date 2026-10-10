#include "os/JoypadMsgs.h"
#include "os/Joypad.h"

ButtonDownMsg::ButtonDownMsg(LocalUser *iUser, JoypadButton iRawButton, JoypadAction iAction, int iPadNum)
    : Message(Type(), iUser, iRawButton, iAction, iPadNum) {}

LocalUser *ButtonDownMsg::GetUser() const { return mData->Obj<LocalUser>(2); }

ButtonUpMsg::ButtonUpMsg(LocalUser *iUser, JoypadButton iRawButton, JoypadAction iAction, int iPadNum)
    : Message(Type(), iUser, iRawButton, iAction, iPadNum) {}

LocalUser *ButtonUpMsg::GetUser() const { return mData->Obj<LocalUser>(2); }

JoypadConnectionMsg::JoypadConnectionMsg(LocalUser *user, bool iConnected, int iType, int padNum)
    : Message(Type(), user, iConnected, iType, padNum) {}

LocalUser *JoypadConnectionMsg::GetUser() const { return mData->Obj<LocalUser>(2); }

JoypadBreedDataReadMsg::JoypadBreedDataReadMsg(LocalUser *user, JoypadBreedDataStatus status)
    : Message(Type(), user, status) {}

JoypadBreedDataWriteMsg::JoypadBreedDataWriteMsg(LocalUser *user, JoypadBreedDataStatus status)
    : Message(Type(), user, status) {}
