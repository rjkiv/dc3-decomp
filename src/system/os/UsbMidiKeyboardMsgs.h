#pragma once
#include "obj/Msg.h"

DECLARE_MESSAGE(KeyboardKeyPressedMsg, "keyboard_key_pressed");
KeyboardKeyPressedMsg(int keyNumber, int keyVel, int padNum);
int GetKeyNum() const { return mData->Int(2); }
int GetKeyVel() const { return mData->Int(3); }
int GetPadNum() const { return mData->Int(4); }
END_MESSAGE

DECLARE_MESSAGE(KeyboardKeyReleasedMsg, "keyboard_key_released")
KeyboardKeyReleasedMsg(int keyNumber, int padNum);
int GetKeyNum() const { return mData->Int(2); }
int GetPadNum() const { return mData->Int(3); }
END_MESSAGE

DECLARE_MESSAGE(KeyboardModMsg, "keyboard_mod")
KeyboardModMsg(int modVal, int padNum);
int GetModVal() const { return mData->Int(2); }
int GetPadNum() const { return mData->Int(3); }
END_MESSAGE

DECLARE_MESSAGE(KeyboardExpressionPedalMsg, "keyboard_expression_pedal")
KeyboardExpressionPedalMsg(int expressionPedal, int padNum);
END_MESSAGE

DECLARE_MESSAGE(KeyboardConnectedAccessoriesMsg, "keyboard_connected_accessories")
KeyboardConnectedAccessoriesMsg(int connectedAccessories, int padNum);
END_MESSAGE

DECLARE_MESSAGE(KeyboardSustainMsg, "keyboard_sustain")
KeyboardSustainMsg(bool sustain, int padNum);
bool GetSustain() const { return mData->Int(2); }
int GetPadNum() const { return mData->Int(3); }
END_MESSAGE

DECLARE_MESSAGE(KeyboardStompBoxMsg, "keyboard_stomp_box")
KeyboardStompBoxMsg(bool stompBox, int padNum);
END_MESSAGE

DECLARE_MESSAGE(KeysAccelerometerMsg, "keys_accelerometer")
KeysAccelerometerMsg(int accelX, int accelY, int accelZ, int padNum);
END_MESSAGE

DECLARE_MESSAGE(KeyboardLowHandPlacementMsg, "keyboard_low_hand_placement")
KeyboardLowHandPlacementMsg(int keyNum, int padNum);
END_MESSAGE

DECLARE_MESSAGE(KeyboardHighHandPlacementMsg, "keyboard_high_hand_placement")
KeyboardHighHandPlacementMsg(int keyNum, int padNum);
END_MESSAGE
