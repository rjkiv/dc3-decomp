#include "os/UsbMidiGuitarMsgs.h"

StringStrummedMsg::StringStrummedMsg(int stringNum, int fretNum, int velocity, int iPadNum)
    : Message(Type(), stringNum, fretNum, velocity, iPadNum) {}

RGAccelerometerMsg::RGAccelerometerMsg(int accelX, int accelY, int accelZ, int iPadNum)
    : Message(Type(), accelX, accelY, accelZ, iPadNum) {}

RGConnectedAccessoriesMsg::RGConnectedAccessoriesMsg(int connectedAccessories, int iPadNum)
    : Message(Type(), connectedAccessories, iPadNum) {}

RGPitchBendMsg::RGPitchBendMsg(int pitchBend, int iPadNum) : Message(Type(), pitchBend, iPadNum) {}

RGMutingMsg::RGMutingMsg(int muting, int iPadNum) : Message(Type(), muting, iPadNum) {}

RGStompBoxMsg::RGStompBoxMsg(bool stompBox, int iPadNum) : Message(Type(), stompBox, iPadNum) {}

RGProgramChangeMsg::RGProgramChangeMsg(int programChange, int iPadNum) : Message(Type(), programChange, iPadNum) {}

RGSwingMsg::RGSwingMsg(int swingMask, int iPadNum) : Message(Type(), swingMask, iPadNum) {}

RGFretButtonDownMsg::RGFretButtonDownMsg(int fret, int iPadNum, bool shifted)
    : Message(Type(), fret, iPadNum, shifted) {}

RGFretButtonUpMsg::RGFretButtonUpMsg(int fret, int iPadNum, bool shifted)
    : Message(Type(), fret, iPadNum, shifted) {}

StringStoppedMsg::StringStoppedMsg(int i1, int i2, int i3, int i4)
    : Message(Type(), i1, i2, i3, i4) {}
