#include "os/UsbMidiKeyboard.h"
#include "os/UsbMidiKeyboardMsgs.h"

bool UsbMidiKeyboard::mUsbMidiKeyboardExists = false;
namespace {
    bool gUseMidiPort = false;
    bool gForceDetectKeytar = false;
}
UsbMidiKeyboard *TheKeyboard;

void UsbMidiKeyboard::Poll() {
    if (!gUseMidiPort && TheKeyboard) {
        for (int i = 0; i < 4; i++) {
            JoypadType ty = JoypadGetPadData(i)->mType;
            if (ty == kJoypadXboxMidiBoxKeyboard || ty == kJoypadPs3MidiBoxKeyboard
                || ty == kJoypadWiiMidiBoxKeyboard || ty == kJoypadXboxKeytar
                || ty == kJoypadPs3Keytar || ty == kJoypadWiiKeytar
                || gForceDetectKeytar) {
                unsigned char *extended = JoypadGetPadData(i)->GetExtended();
                // this loop sets keys as pressed or released
                int ivar1 = 1;
                for (int j = 0; j < 25; j++) {
                    int u5 = extended[j];
                    if (u5 == TheKeyboard->GetKeyPressed(i, j)) {
                        if (u5 != 0)
                            ivar1++;
                    } else {
                        if (u5 != 0) {
                            int extVel = TheKeyboard->GetSlottedKeyVelocityFromExtended(
                                ivar1++, extended
                            );
                            TheKeyboard->SetKeyVelocity(i, j, extVel);
                            KeyboardKeyPressedMsg msg(
                                j, TheKeyboard->GetKeyVelocity(i, j), i
                            );
                            JoypadPushThroughMsg(msg);
                        } else {
                            TheKeyboard->SetKeyVelocity(i, j, 0);
                            KeyboardKeyReleasedMsg msg(j, i);
                            JoypadPushThroughMsg(msg);
                        }
                        TheKeyboard->SetKeyPressed(i, j, u5);
                    }
                }
                bool sus = extended[1] & 0x80;
                if (sus != TheKeyboard->GetSustain(i)) {
                    TheKeyboard->SetSustain(i, sus);
                    KeyboardSustainMsg msg(sus, i);
                    JoypadPushThroughMsg(msg);
                }
                bool stomped = extended[2] & 0x80;
                if (stomped != TheKeyboard->GetStompPedal(i)) {
                    TheKeyboard->SetStompPedal(i, stomped);
                    KeyboardStompBoxMsg msg(stomped, i);
                    JoypadPushThroughMsg(msg);
                }
                int mod = extended[0xa] & 0x7F;
                if (mod != TheKeyboard->GetModVal(i)) {
                    TheKeyboard->SetModVal(i, mod);
                    KeyboardModMsg msg(mod, i);
                    JoypadPushThroughMsg(msg);
                }
                int exp = extended[9] & 0x7F;
                if (exp != TheKeyboard->GetExpressionPedal(i)) {
                    TheKeyboard->SetExpressionPedal(i, exp);
                    KeyboardExpressionPedalMsg msg(exp, i);
                    JoypadPushThroughMsg(msg);
                }
                int conn = extended[0xf];
                if (conn != TheKeyboard->GetConnectedAccessory(i)) {
                    TheKeyboard->SetConnectedAccessories(i, conn);
                    KeyboardConnectedAccessoriesMsg msg(conn, i);
                    JoypadPushThroughMsg(msg);
                }
                int lowhand = extended[0xe] & 0x1F;
                if (lowhand != TheKeyboard->GetLowHandPlacement(i)) {
                    TheKeyboard->SetLowHandPlacement(i, lowhand);
                    KeyboardLowHandPlacementMsg msg(lowhand, i);
                    JoypadPushThroughMsg(msg);
                }
                int highhand = (extended[0xb] >> 7 & 1) + ((extended[0xc] >> 7 & 1) << 1)
                    + ((extended[0xd] >> 7 & 1) << 2) + ((extended[0xe] >> 5 & 0x3) << 3);
                if (highhand != TheKeyboard->GetHighHandPlacement(i)) {
                    TheKeyboard->SetHighHandPlacement(i, highhand);
                    KeyboardHighHandPlacementMsg msg(highhand, i);
                    JoypadPushThroughMsg(msg);
                }
                int accelaxisval0 = extended[0xa] & 0x7F;
                int accelaxisval1 = extended[0xb] & 0x7F;
                int accelaxisval2 = extended[0xc] & 0x7F;
                if (accelaxisval0 != TheKeyboard->GetAccelAxisVal(i, 0)
                    || accelaxisval1 != TheKeyboard->GetAccelAxisVal(i, 1)
                    || accelaxisval2 != TheKeyboard->GetAccelAxisVal(i, 2)) {
                    TheKeyboard->SetAccelerometer(
                        i, accelaxisval0, accelaxisval1, accelaxisval2
                    );
                    KeysAccelerometerMsg msg(
                        accelaxisval0, accelaxisval1, accelaxisval2, i
                    );
                    JoypadPushThroughMsg(msg);
                }
            }
        }
    }
}

int UsbMidiKeyboard::GetSlottedKeyVelocityFromExtended(int i, unsigned char *uc) {
    if (gUseMidiPort)
        return 0;
    if (i >= 1 && i <= 5) {
        switch (i) {
        case 1:
            return uc[3] & 0x7F;
        case 2:
            return uc[4] & 0x7F;
        case 3:
            return uc[5] & 0x7F;
        case 4:
            return uc[6] & 0x7F;
        case 5:
            return uc[7] & 0x7F;
        }
    }
    return 0;
}

int UsbMidiKeyboard::GetAccelAxisVal(int pad, int axis) {
    if (0 <= axis && (unsigned int)axis < 4) {
        return mAccelerometer[pad][axis];
    }
    return 0;
}

bool UsbMidiKeyboard::GetKeyPressed(int pad, int key) {
    if ((unsigned int)key <= 0x7F) {
        return mKeyPressed[pad][key];
    }
    return 0;
}

int UsbMidiKeyboard::GetKeyVelocity(int pad, int vel) {
    if ((unsigned int)vel <= 0x7F) {
        return mKeyVelocity[pad][vel];
    }
    return 0;
}

void UsbMidiKeyboard::SetAccelerometer(int pad, int a1, int a2, int a3) {
    mAccelerometer[pad][0] = a1;
    mAccelerometer[pad][1] = a2;
    mAccelerometer[pad][2] = a3;
}

void UsbMidiKeyboard::SetSustain(int pad, bool sus) { mSustain[pad] = sus; }

void UsbMidiKeyboard::SetStompPedal(int pad, bool stomp) { mStompPedal[pad] = stomp; }

void UsbMidiKeyboard::SetModVal(int pad, int mod) { mModVal[pad] = mod; }

void UsbMidiKeyboard::SetExpressionPedal(int pad, int exp) {
    mExpressionPedal[pad] = exp;
}

void UsbMidiKeyboard::SetConnectedAccessories(int pad, int conn) {
    mConnectedAccessories[pad] = conn;
}

void UsbMidiKeyboard::SetLowHandPlacement(int pad, int lh) {
    mLowHandPlacement[pad] = lh;
}

void UsbMidiKeyboard::SetHighHandPlacement(int pad, int hh) {
    mHighHandPlacement[pad] = hh;
}

void UsbMidiKeyboard::SetKeyPressed(int pad, int key, bool pressed) {
    if (0x7F < (unsigned int)key)
        return;
    mKeyPressed[pad][key] = pressed;
}

void UsbMidiKeyboard::SetKeyVelocity(int pad, int key, int vel) {
    if (0x7F < (unsigned int)key)
        return;
    mKeyVelocity[pad][key] = vel;
}
