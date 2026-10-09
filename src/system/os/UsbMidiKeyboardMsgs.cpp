#include "os/UsbMidiKeyboardMsgs.h"

KeyboardKeyPressedMsg::KeyboardKeyPressedMsg(int keyNumber, int keyVel, int iPadNum)
    : Message(Type(), keyNumber, keyVel, iPadNum) {}

KeyboardKeyReleasedMsg::KeyboardKeyReleasedMsg(int keyNumber, int iPadNum)
    : Message(Type(), keyNumber, iPadNum) {}

KeyboardModMsg::KeyboardModMsg(int modVal, int iPadNum) : Message(Type(), modVal, iPadNum) {}

KeyboardExpressionPedalMsg::KeyboardExpressionPedalMsg(int expressionPedal, int iPadNum)
    : Message(Type(), expressionPedal, iPadNum) {}

KeyboardConnectedAccessoriesMsg::KeyboardConnectedAccessoriesMsg(int connectedAccessories, int iPadNum)
    : Message(Type(), connectedAccessories, iPadNum) {}

KeyboardSustainMsg::KeyboardSustainMsg(bool sustain, int iPadNum) : Message(Type(), sustain, iPadNum) {}

KeyboardStompBoxMsg::KeyboardStompBoxMsg(bool stompBox, int iPadNum) : Message(Type(), stompBox, iPadNum) {}

KeysAccelerometerMsg::KeysAccelerometerMsg(int accelX, int accelY, int accelZ, int iPadNum)
    : Message(Type(), accelX, accelY, accelZ, iPadNum) {}

KeyboardLowHandPlacementMsg::KeyboardLowHandPlacementMsg(int keyNum, int iPadNum)
    : Message(Type(), keyNum, iPadNum) {}

KeyboardHighHandPlacementMsg::KeyboardHighHandPlacementMsg(int keyNum, int iPadNum)
    : Message(Type(), keyNum, iPadNum) {}
