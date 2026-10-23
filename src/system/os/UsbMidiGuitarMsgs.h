#pragma once
#include "obj/Msg.h"

DECLARE_MESSAGE(StringStrummedMsg, "string_strummed")
StringStrummedMsg(int stringNum, int fretNum, int velocity, int padNum);
int GetStringStrummed() const { return mData->Int(2); }
int GetFretNum() const { return mData->Int(3); }
int GetVelocity() const { return mData->Int(4); }
int GetPadNum() const { return mData->Int(5); }
END_MESSAGE

DECLARE_MESSAGE(StringStoppedMsg, "string_stopped")
StringStoppedMsg(int, int, int, int);
int GetStringNum() const { return mData->Int(2); }
int GetFretNum() const { return mData->Int(3); }
int GetVelocity() const { return mData->Int(4); }
int GetPadNum() const { return mData->Int(5); }
END_MESSAGE

DECLARE_MESSAGE(RGAccelerometerMsg, "rg_accelerometer")
RGAccelerometerMsg(int accelX, int accelY, int accelZ, int padNum);
int GetAccelX() const { return mData->Int(2); }
int GetAccelY() const { return mData->Int(3); }
int GetAccelZ() const { return mData->Int(4); }
int GetPadNum() const { return mData->Int(5); }
END_MESSAGE

DECLARE_MESSAGE(RGConnectedAccessoriesMsg, "rg_connected_accessories")
RGConnectedAccessoriesMsg(int connectedAccessories, int padNum);
END_MESSAGE

DECLARE_MESSAGE(RGPitchBendMsg, "rg_pitch_bend")
RGPitchBendMsg(int pitchBend, int padNum);
END_MESSAGE

DECLARE_MESSAGE(RGMutingMsg, "rg_muting")
RGMutingMsg(int muting, int padNum);
END_MESSAGE

DECLARE_MESSAGE(RGStompBoxMsg, "rg_stomp_box")
RGStompBoxMsg(bool stompBox, int padNum);
END_MESSAGE

DECLARE_MESSAGE(RGProgramChangeMsg, "rg_program_change")
RGProgramChangeMsg(int programChange, int padNum);
END_MESSAGE

DECLARE_MESSAGE(RGSwingMsg, "rg_swing")
RGSwingMsg(int swingMask, int padNum);
int GetSwingMask() const { return mData->Int(2); }
int GetPadNum() const { return mData->Int(3); }
END_MESSAGE

DECLARE_MESSAGE(RGFretButtonDownMsg, "rg_fret_button_down")
RGFretButtonDownMsg(int fret, int padNum, bool shifted);
int GetFret() const { return mData->Int(2); }
int GetPadNum() const { return mData->Int(3); }
int GetShifted() const { return mData->Int(4); }
END_MESSAGE

DECLARE_MESSAGE(RGFretButtonUpMsg, "rg_fret_button_up")
RGFretButtonUpMsg(int fret, int padNum, bool shifted);
int GetFret() const { return mData->Int(2); }
int GetPadNum() const { return mData->Int(3); }
int GetShifted() const { return mData->Int(4); }
END_MESSAGE
