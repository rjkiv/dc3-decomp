#include "os/UsbMidiGuitar.h"
#include "os/Joypad.h"
#include "os/UsbMidiGuitarMsgs.h"

CriticalSection gCritSection;
Queue gQueue(0x32);
Timer UsbMidiGuitar::mTimer;
UsbMidiGuitar *TheGuitar;
bool UsbMidiGuitar::mUsbMidiGuitarExists = false;
int UsbMidiGuitar::mMinVelocity = 5;

void UsbMidiGuitar::Poll() {
    if (TheGuitar) {
        for (int i = 0; i < 4; i++) {
            JoypadType ty = JoypadGetPadData(i)->mType;
            if (ty == kJoypadXboxButtonGuitar || ty == kJoypadPs3ButtonGuitar
                || ty == kJoypadWiiButtonGuitar || ty == kJoypadXboxRealGuitar22Fret
                || ty == kJoypadPs3RealGuitar22Fret || ty == kJoypadWiiRealGuitar22Fret) {
                unsigned char *extended = JoypadGetPadData(i)->GetExtended();
                unsigned int uVar9 = 0;
                for (int j = 5; j >= 0; j--) {
                    int curFret, curVel;
                    switch (j) {
                    case 5:
                        curFret = extended[0] & 0x1F;
                        curVel = extended[4] & 0x7F;
                        break;
                    case 4:
                        curFret = ((extended[1] & 3) << 3) + (extended[0] >> 5);
                        curVel = extended[5] & 0x7F;
                        break;
                    case 3:
                        curFret = extended[1] >> 2 & 0x1F;
                        curVel = extended[6] & 0x7F;
                        break;
                    case 2:
                        curFret = extended[2] & 0x1F;
                        curVel = extended[7] & 0x7F;
                        break;
                    case 1:
                        curFret = ((extended[3] & 3) << 3) + (extended[2] >> 5);
                        curVel = extended[8] & 0x7F;
                        break;
                    default:
                        curFret = extended[3] >> 2 & 0x1F;
                        curVel = extended[9] & 0x7F;
                        break;
                    }
                    bool velUpdated = false;
                    if (curVel != TheGuitar->GetVelocity(i, j)) {
                        TheGuitar->SetVelocity(i, j, curVel);
                        velUpdated = true;
                    }
                    if (curFret != TheGuitar->GetFret(i, j) || velUpdated) {
                        TheGuitar->SetFret(i, j, curFret);
                        TheGuitar->SetVelocity(i, j, curVel);
                        StringStrummedMsg ssmsg(j, curFret, velUpdated ? curVel : 0, i);
                        JoypadPushThroughMsg(ssmsg);
                        TheGuitar->UpdateStringStrummed(i, j);
                        if ((curVel > UsbMidiGuitar::mMinVelocity) && velUpdated) {
                            uVar9 |= 1 << (5 - j);
                        }
                    }
                }
                bool someCharArr[6];
                bool RGFretBool = extended[3] & 0x80;
                for (int k = 0; k < 5; k++) {
                    bool padDataFretDown = extended[k + 4] & 0x80;
                    someCharArr[k] = padDataFretDown;
                    if (padDataFretDown != TheGuitar->GetFretDown(i, k)) {
                        if (padDataFretDown) {
                            TheGuitar->SetFretDown(i, k, true);
                            RGFretButtonDownMsg msg(k, i, RGFretBool);
                            JoypadPushThroughMsg(msg);
                        } else {
                            TheGuitar->SetFretDown(i, k, false);
                            RGFretButtonUpMsg msg(k, i, RGFretBool);
                            JoypadPushThroughMsg(msg);
                        }
                    }
                }
                if (uVar9 != 0) {
                    RGSwingMsg msg(uVar9, i);
                    JoypadPushThroughMsg(msg);
                }
                int axisVal11 = extended[0xa] & 0x7F;
                int axisVal12 = extended[0xb] & 0x7F;
                int axisVal10 = extended[0xc] & 0x7F;
                if (axisVal11 != TheGuitar->CurrentAccelAxisVal(i, 0)
                    || axisVal12 != TheGuitar->CurrentAccelAxisVal(i, 1)
                    || axisVal10 != TheGuitar->CurrentAccelAxisVal(i, 2)) {
                    TheGuitar->SetAccelerometer(i, axisVal11, axisVal12, axisVal10);
                    RGAccelerometerMsg msg(axisVal11, axisVal12, axisVal10, i);
                    JoypadPushThroughMsg(msg);
                }
                int connAcc = extended[0xd] & 0x7F;
                if (connAcc != TheGuitar->GetConnectedAccessory(i)) {
                    TheGuitar->SetConnectedAccessories(i, connAcc);
                    RGConnectedAccessoriesMsg msg(connAcc, i);
                    JoypadPushThroughMsg(msg);
                }
                int pitchBend = extended[0xd] & 0x7F;
                if (pitchBend != TheGuitar->GetPitchBend(i)) {
                    TheGuitar->SetPitchBend(i, pitchBend);
                    RGPitchBendMsg msg(pitchBend, i);
                    JoypadPushThroughMsg(msg);
                }
                int muting = extended[0xe] & 0x7F;
                if (muting != TheGuitar->GetMuting(i)) {
                    TheGuitar->SetMuting(i, muting);
                    RGMutingMsg msg(muting, i);
                    JoypadPushThroughMsg(msg);
                }
                bool stompBox = extended[0xd] & 0x80;
                if (stompBox != TheGuitar->GetStompBox(i)) {
                    TheGuitar->SetStompBox(i, stompBox);
                    RGStompBoxMsg msg(stompBox, i);
                    JoypadPushThroughMsg(msg);
                }
                int programChange = (extended[0xa] >> 7 & 1)
                    + ((extended[0xb] >> 7 & 1) << 1) + ((extended[0xc] >> 7 & 1) << 2);
                if (programChange != TheGuitar->GetProgramChange(i)) {
                    TheGuitar->SetProgramChange(i, programChange);
                    RGProgramChangeMsg msg(programChange, i);
                    JoypadPushThroughMsg(msg);
                }
            }
        }
    }
}

// axes are presumably X, Y, Z
int UsbMidiGuitar::CurrentAccelAxisVal(int pad, int axis) {
    if (0 <= axis && (unsigned int)axis < 4)
        return mAccelerometer[pad][axis];
    else
        return 0;
}

void UsbMidiGuitar::SetFret(int pad, int str, int fret) { mStringFret[pad][str] = fret; }

void UsbMidiGuitar::SetVelocity(int pad, int str, int vel) {
    mStringVelocity[pad][str] = vel;
}

void UsbMidiGuitar::SetAccelerometer(int pad, int a1, int a2, int a3) {
    mAccelerometer[pad][0] = a1;
    mAccelerometer[pad][1] = a2;
    mAccelerometer[pad][2] = a3;
}

void UsbMidiGuitar::SetConnectedAccessories(int pad, int accs) {
    mConnectedAccessories[pad] = accs;
}

void UsbMidiGuitar::SetPitchBend(int pad, int pb) { mPitchBend[pad] = pb; }

void UsbMidiGuitar::SetMuting(int pad, int mute) { mMuting[pad] = mute; }

void UsbMidiGuitar::SetStompBox(int pad, bool stomp) { mStompBox[pad] = stomp; }

void UsbMidiGuitar::SetProgramChange(int pad, int pc) { mProgramChange[pad] = pc; }

void UsbMidiGuitar::SetFretDown(int pad, int str, bool down) {
    mFretDown[pad][str] = down;
}

void UsbMidiGuitar::UpdateStringStrummed(int pad, int str) {
    for (int i = 6; i > 0; i--) {
        mLastSixStringsStrummed[pad][i] = mLastSixStringsStrummed[pad][i - 1];
    }
    mLastSixStringsStrummed[pad][0] = str;
}

Queue::Queue(int i) : mArrayStart(0) { Initialize(i); }

Queue::~Queue() {
    CritSecTracker tracker(&gCritSection);
    delete mArrayStart;
}

void Queue::Initialize(int i) {
    CritSecTracker tracker(&gCritSection);
    delete mArrayStart;
    mArrayStart = new MidiMessage[i + 1];
    mArrayEnd = &mArrayStart[i] + 1;
    mUsurpedFret[0] = mUsurpedFret[1] = mUsurpedFret[2] = mUsurpedFret[3] =
        mUsurpedFret[4] = mUsurpedFret[5] = -1;
    mQueueStart = mArrayStart;
    mQueueEnd = mArrayStart;
}
