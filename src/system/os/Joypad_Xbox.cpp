#include "os/Joypad_Xbox.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/Joypad_Xinput.h"
#include "os/System.h"
#include "xdk/XAPILIB.h"

namespace {
    BreedData tBreed[kNumJoypads];
    HANDLE tThread;
    bool tNoHandle;
    XINPUT_STATE tInputStates[kNumJoypads];
    CriticalSection tCritSection;
}

void GetXinputSinceLastFrame(int pad, XINPUT_STATE *state, unsigned int *ui3) {
    CritSecTracker tracker(&tCritSection);
    *state = tInputStates[pad];
    unsigned int x;
    TranslateButtons(&x, tInputStates[pad].Gamepad.wButtons);
}

void XinputJoypadThreadDestruction() {
    tNoHandle = true;
    WaitForSingleObject(tThread, -1);
    CloseHandle(tThread);
    tThread = nullptr;
}

void JoypadReset() { JoypadResetXboxPC(4); }

void JoypadTerminate() {
    XinputJoypadThreadDestruction();
    JoypadTerminateCommon();
}

void JoypadPoll() { JoypadPollCommon(); }

JoypadType SetupHXKeytar(int, const XINPUT_CAPABILITIES &c) {
    if ((c.Gamepad.sThumbLY & 0xFFF0U) == 0x1730) {
        return kJoypadXboxMidiBoxKeyboard;
    } else
        return kJoypadXboxKeytar;
}

void ReceiveUpstreamLowPriorityOutputResponse(int pad, unsigned char *data) {
    MILO_LOG("Low Priority Output Report for controller %d:\n", pad);
    MILO_LOG("0x%02x 0x%02x 0x%02x\n", data[1], data[2], data[3]);
}

void ReceiveUpstreamBreedDataResponse(int pad, unsigned char *data) {
    if (JoypadGetPadData(pad)->mConnected) {
        MILO_LOG("Breed Data Response for controller %d\n", pad);
        MILO_LOG(
            "Vendor:      0x%02x\nProject:     0x%02x\nPeriph Type: 0x%02x\nPlatform:    0x%02x\nFactory:     0x%02x\nDesign Iter: 0x%02x\nManu Date(1):0x%02x\nManu Date(2):0x%02x\nIdent. v(1): 0x%02x\nIdent. v(2): 0x%02x\n",
            data[1],
            data[2],
            data[3],
            data[4],
            data[5],
            data[6],
            data[7],
            data[8],
            data[9],
            data[10]
        );
    }
    tBreed[pad].bVendor = data[1];
    tBreed[pad].bProject = data[2];
    tBreed[pad].bPeriphType = data[3];
    tBreed[pad].bPlatform = data[4];
    tBreed[pad].bFactory = data[5];
    tBreed[pad].bDesignIter = data[6];
    tBreed[pad].uManuDate = data[8] * 0x100 + data[7];
    tBreed[pad].uUnique = data[10] * 0x100 + data[9];
    tBreed[pad].bUninitialized = false;
    JoypadHandleBreedDataResponse(pad);
}

void ReceiveUpstreamCalbertResponse(int pad, unsigned char *data) {
    MILO_LOG("Calbert Response for controller %d\n", pad);
    MILO_LOG("Sensor Output Mode: 0x%02x\n", data[1]);
}

void ReceiveUpstreamAccelerometerResponse(int pad, unsigned char *data) {
    MILO_LOG("Accelerometer Mode Response for controller %d\n", pad);
    MILO_LOG(
        "Accelerometer Output Mode: 0x%02x\nX axis resolution:         0x%02x\nY axis resolution:         0x%02x\nZ axis resolution:         0x%02x\n",
        data[1],
        data[2],
        data[3],
        data[4]
    );
}

void ReceiveUpstreamOutputModeResponse(int pad, unsigned char *data) {
    MILO_LOG("Output Mode Switch Response for controller %d\n", pad);
    MILO_LOG("Output Mode: 0x%02x\n", data[1]);
}

void ReceiveUpstreamDeviceStateResponse(int pad, unsigned char *data) {
    MILO_LOG("Device State Response for controller %d\n", pad);
    MILO_LOG("Battery Level: 0x%02x\nOutput Mode:   0x%02x\n", data[1], data[2]);
}

void ReceiveUpstreamEEPROMReadResponse(int pad, unsigned char *data) {
    MILO_LOG("EEPROM Read Response for controller %d\n", pad);
    MILO_LOG(
        "Offset (low):      0x%02x\nOffset (high):     0x%02x\nData Length:       0x%02x\n",
        data[1],
        data[2],
        data[3]
    );
    MILO_LOG(
        "Packet Payload Len:0x%02x\nEEPROM Data(1):    0x%02x\nEEPROM Data(2):    0x%02x\nEEPROM Data(3):    0x%02x\nEEPROM Data(4):    0x%02x\nEEPROM Data(5):    0x%02x\nEEPROM Data(6):    0x%02x\nEEPROM Data(7):    0x%02x\nEEPROM Data(8):    0x%02x\n",
        data[5],
        data[6],
        data[7],
        data[8],
        data[9],
        data[10],
        data[11],
        data[12],
        data[13]
    );
}

void ReceiveUpstreamEEPROMWriteResponse(int pad, unsigned char *data) {
    MILO_LOG("EEPROM Write Response for controller %d\n", pad);
    MILO_LOG(
        "Offset (low):       0x%02x\nOffset (high):      0x%02x\nData Length:        0x%02x\nStatus:             0x%02x\n",
        data[1],
        data[2],
        data[3],
        data[4]
    );
    MILO_LOG(
        "Packet Payload Len: 0x%02x\nEEPROM Data Echo(1):0x%02x\nEEPROM Data Echo(2):0x%02x\nEEPROM Data Echo(3):0x%02x\nEEPROM Data Echo(4):0x%02x\nEEPROM Data Echo(5):0x%02x\nEEPROM Data Echo(6):0x%02x\nEEPROM Data Echo(7):0x%02x\nEEPROM Data Echo(8):0x%02x\n",
        data[5],
        data[6],
        data[7],
        data[8],
        data[9],
        data[10],
        data[11],
        data[12],
        data[13]
    );
    JoypadHandleEepromWriteResponse(pad, (JoypadBreedDataStatus)(data[4] != 0));
}

void SendRawData(
    int,
    unsigned char,
    unsigned char,
    unsigned char,
    unsigned char,
    unsigned char,
    unsigned char,
    unsigned char
);

BreedData *GetBreedData(int pad) {
    if (tBreed[pad].bUninitialized) {
        SendRawData(pad, 0x81, 0, 0, 0, 0, 0, 0);
        return nullptr;
    } else {
        return &tBreed[pad];
    }
}

bool requestBreedWrite(int pad, unsigned char *pBreedWritePacket) {
    MILO_ASSERT(pBreedWritePacket, 0x301);
    SendRawData(
        pad,
        0xF3,
        pBreedWritePacket[0],
        pBreedWritePacket[1],
        pBreedWritePacket[2],
        pBreedWritePacket[3],
        pBreedWritePacket[4],
        pBreedWritePacket[5]
    );
    return true;
}

JoypadType SetupHXRealGuitar(int pad, const XINPUT_CAPABILITIES &c) {
    short us = c.Gamepad.sThumbLY;
    bool u1 = us == 0x1530;
    bool u2 = us == 0x1430;
    if (!u1 && !u2)
        u2 = true;
    if (u1) {
        return kJoypadXboxRealGuitar22Fret;
    } else if (u2) {
        return kJoypadXboxButtonGuitar;
    } else {
        MILO_LOG("sThymbLY = %d does not correspond to subtype x19\n", c.Gamepad.sThumbLY);
        return kJoypadAnalog;
    }
}

JoypadType SetupHXGuitar(int pad, const XINPUT_CAPABILITIES &c) {
    bool u5 = c.Flags & 0x2;
    bool u1 = c.Flags & 1;
    bool u4 = u5 && (u1 || c.Gamepad.sThumbRX >= 0x100);
    JoypadGetPadData(pad)->SetWireless(u5);
    JoypadGetPadData(pad)->SetCanForceFeedback(u1);
    if (c.Gamepad.sThumbLX == 0x1BAD) {
        GetBreedData(pad);
        return kJoypadXboxCoreGuitar;
    } else
        return u4 ? kJoypadXboxHxGuitarRb2 : kJoypadXboxHxGuitar;
}

JoypadType SetupHXDrums(int pad, const XINPUT_CAPABILITIES &c) {
    bool u5 = c.Flags & 0x2;
    bool u1 = c.Flags & 1;
    bool u4 = u5 && (u1 || c.Gamepad.sThumbRX >= 0x100);
    bool u2 = u5 && u1;
    JoypadGetPadData(pad)->SetWireless(u5);
    JoypadGetPadData(pad)->SetCanForceFeedback(u1);
    if (c.Gamepad.sThumbLX == 0x1BAD) {
        GetBreedData(pad);
        return kJoypadXboxMidiBoxDrums;
    } else if (u4) {
        return kJoypadXboxDrumsRb2;
    } else
        return u2 ? kJoypadXboxRoDrums : kJoypadXboxDrums;
}

bool ReceiveUpstreamResponse(int pad, unsigned char *data) {
    switch (data[0]) {
    case 0x80:
        ReceiveUpstreamLowPriorityOutputResponse(pad, data);
        break;
    case 0x82:
        ReceiveUpstreamBreedDataResponse(pad, data);
        break;
    case 0x84:
        ReceiveUpstreamCalbertResponse(pad, data);
        break;
    case 0x86:
        ReceiveUpstreamAccelerometerResponse(pad, data);
        break;
    case 0x8A:
        ReceiveUpstreamOutputModeResponse(pad, data);
        break;
    case 0xC4:
        ReceiveUpstreamDeviceStateResponse(pad, data);
        break;
    case 0xF2:
        ReceiveUpstreamEEPROMReadResponse(pad, data);
        break;
    case 0xF4:
        ReceiveUpstreamEEPROMWriteResponse(pad, data);
        break;
    default:
        return false;
    }
    return true;
}

namespace {
    void InitXinputJoypadThreadData();

    void RunXinputJoypadLoop();

    DWORD XinputJoypadThreadEntry(HANDLE) {
        InitXinputJoypadThreadData();
        RunXinputJoypadLoop();
        return 0;
    }
}

void XinputJoypadThreadStart() {
    tThread = CreateThread(nullptr, 0, XinputJoypadThreadEntry, nullptr, 4, nullptr);
    MILO_ASSERT(tThread, 0x266);
    SetThreadPriority(tThread, 2);
    XSetThreadProcessor(tThread, 1);
    ResumeThread(tThread);
}

void JoypadInit() {
    DataArray *cfg = SystemConfig("joypad");
    JoypadInitCommon(cfg);
    JoypadInitXboxPCDeadzone(cfg);
    JoypadReset();
    XinputJoypadThreadStart();
}
