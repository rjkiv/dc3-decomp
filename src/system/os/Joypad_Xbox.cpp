#include "os/Joypad_Xbox.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/Joypad_Xinput.h"
#include "os/System.h"
#include "xdk/XAPILIB.h"

namespace {
    static XINPUT_STATE tXInputStates[kNumJoypads] = { 0 };
    BreedData tBreed[kNumJoypads];
    static HANDLE tThread = nullptr;
    static bool tNoHandle = false;
    static unsigned int unkc8[4] = { 0 };
    static unsigned int unkd8[4] = { 0 };
    CriticalSection tCritSection;

    void InitXinputJoypadThreadData();
    void RunXinputJoypadLoop();

    DWORD XinputJoypadThreadEntry(HANDLE) {
        InitXinputJoypadThreadData();
        RunXinputJoypadLoop();
        return 0;
    }
}

void GetXinputSinceLastFrame(int xinput_id, XINPUT_STATE *state, unsigned int *buttons) {
    CritSecTracker tracker(&tCritSection);
    *state = tXInputStates[xinput_id];
    unsigned int x;
    TranslateButtons(&x, tXInputStates[xinput_id].Gamepad.wButtons);
    *buttons = x | unkc8[xinput_id];
    unkd8[xinput_id] = unkc8[xinput_id];
    unkc8[xinput_id] = 0;
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

JoypadType SetupHXKeytar(int iPadNum, const XINPUT_CAPABILITIES &xcaps) {
    if ((xcaps.Gamepad.sThumbLY & 0xFFF0U) == 0x1730) {
        return kJoypadXboxMidiBoxKeyboard;
    } else
        return kJoypadXboxKeytar;
}

void ReceiveUpstreamLowPriorityOutputResponse(int xinput_id, unsigned char *usefulRawStart) {
    MILO_LOG("Low Priority Output Report for controller %d:\n", xinput_id);
    MILO_LOG("0x%02x 0x%02x 0x%02x\n", usefulRawStart[1], usefulRawStart[2], usefulRawStart[3]);
}

void ReceiveUpstreamBreedDataResponse(int xinput_id, unsigned char *usefulRawStart) {
    if (JoypadGetPadData(xinput_id)->mConnected) {
        MILO_LOG("Breed Data Response for controller %d\n", xinput_id);
        MILO_LOG(
            "Vendor:      0x%02x\nProject:     0x%02x\nPeriph Type: 0x%02x\nPlatform:    0x%02x\nFactory:     0x%02x\nDesign Iter: 0x%02x\nManu Date(1):0x%02x\nManu Date(2):0x%02x\nIdent. v(1): 0x%02x\nIdent. v(2): 0x%02x\n",
            usefulRawStart[1],
            usefulRawStart[2],
            usefulRawStart[3],
            usefulRawStart[4],
            usefulRawStart[5],
            usefulRawStart[6],
            usefulRawStart[7],
            usefulRawStart[8],
            usefulRawStart[9],
            usefulRawStart[10]
        );
    }
    tBreed[xinput_id].bVendor = usefulRawStart[1];
    tBreed[xinput_id].bProject = usefulRawStart[2];
    tBreed[xinput_id].bPeriphType = usefulRawStart[3];
    tBreed[xinput_id].bPlatform = usefulRawStart[4];
    tBreed[xinput_id].bFactory = usefulRawStart[5];
    tBreed[xinput_id].bDesignIter = usefulRawStart[6];
    tBreed[xinput_id].uManuDate = usefulRawStart[8] * 0x100 + usefulRawStart[7];
    tBreed[xinput_id].uUnique = usefulRawStart[10] * 0x100 + usefulRawStart[9];
    tBreed[xinput_id].bUninitialized = false;
    JoypadHandleBreedDataResponse(xinput_id);
}

void ReceiveUpstreamCalbertResponse(int xinput_id, unsigned char *usefulRawStart) {
    MILO_LOG("Calbert Response for controller %d\n", xinput_id);
    MILO_LOG("Sensor Output Mode: 0x%02x\n", usefulRawStart[1]);
}

void ReceiveUpstreamAccelerometerResponse(int xinput_id, unsigned char *usefulRawStart) {
    MILO_LOG("Accelerometer Mode Response for controller %d\n", xinput_id);
    MILO_LOG(
        "Accelerometer Output Mode: 0x%02x\nX axis resolution:         0x%02x\nY axis resolution:         0x%02x\nZ axis resolution:         0x%02x\n",
        usefulRawStart[1],
        usefulRawStart[2],
        usefulRawStart[3],
        usefulRawStart[4]
    );
}

void ReceiveUpstreamOutputModeResponse(int xinput_id, unsigned char *usefulRawStart) {
    MILO_LOG("Output Mode Switch Response for controller %d\n", xinput_id);
    MILO_LOG("Output Mode: 0x%02x\n", usefulRawStart[1]);
}

void ReceiveUpstreamDeviceStateResponse(int xinput_id, unsigned char *usefulRawStart) {
    MILO_LOG("Device State Response for controller %d\n", xinput_id);
    MILO_LOG("Battery Level: 0x%02x\nOutput Mode:   0x%02x\n", usefulRawStart[1], usefulRawStart[2]);
}

void ReceiveUpstreamEEPROMReadResponse(int xinput_id, unsigned char *usefulRawStart) {
    MILO_LOG("EEPROM Read Response for controller %d\n", xinput_id);
    MILO_LOG(
        "Offset (low):      0x%02x\nOffset (high):     0x%02x\nData Length:       0x%02x\n",
        usefulRawStart[1],
        usefulRawStart[2],
        usefulRawStart[3]
    );
    MILO_LOG(
        "Packet Payload Len:0x%02x\nEEPROM Data(1):    0x%02x\nEEPROM Data(2):    0x%02x\nEEPROM Data(3):    0x%02x\nEEPROM Data(4):    0x%02x\nEEPROM Data(5):    0x%02x\nEEPROM Data(6):    0x%02x\nEEPROM Data(7):    0x%02x\nEEPROM Data(8):    0x%02x\n",
        usefulRawStart[5],
        usefulRawStart[6],
        usefulRawStart[7],
        usefulRawStart[8],
        usefulRawStart[9],
        usefulRawStart[10],
        usefulRawStart[11],
        usefulRawStart[12],
        usefulRawStart[13]
    );
}

void ReceiveUpstreamEEPROMWriteResponse(int xinput_id, unsigned char *usefulRawStart) {
    MILO_LOG("EEPROM Write Response for controller %d\n", xinput_id);
    MILO_LOG(
        "Offset (low):       0x%02x\nOffset (high):      0x%02x\nData Length:        0x%02x\nStatus:             0x%02x\n",
        usefulRawStart[1],
        usefulRawStart[2],
        usefulRawStart[3],
        usefulRawStart[4]
    );
    MILO_LOG(
        "Packet Payload Len: 0x%02x\nEEPROM Data Echo(1):0x%02x\nEEPROM Data Echo(2):0x%02x\nEEPROM Data Echo(3):0x%02x\nEEPROM Data Echo(4):0x%02x\nEEPROM Data Echo(5):0x%02x\nEEPROM Data Echo(6):0x%02x\nEEPROM Data Echo(7):0x%02x\nEEPROM Data Echo(8):0x%02x\n",
        usefulRawStart[5],
        usefulRawStart[6],
        usefulRawStart[7],
        usefulRawStart[8],
        usefulRawStart[9],
        usefulRawStart[10],
        usefulRawStart[11],
        usefulRawStart[12],
        usefulRawStart[13]
    );
    JoypadHandleEepromWriteResponse(xinput_id, (JoypadBreedDataStatus)(usefulRawStart[4] != 0));
}

void SendRawData(
    int xinput_id,
    unsigned char command,
    unsigned char data1,
    unsigned char data2,
    unsigned char data3,
    unsigned char data4,
    unsigned char data5,
    unsigned char data6
);

BreedData *GetBreedData(int iPadNum) {
    if (tBreed[iPadNum].bUninitialized) {
        SendRawData(iPadNum, 0x81, 0, 0, 0, 0, 0, 0);
        return nullptr;
    } else {
        return &tBreed[iPadNum];
    }
}

bool requestBreedWrite(int iPadNum, unsigned char *pBreedWritePacket) {
    MILO_ASSERT(pBreedWritePacket, 0x301);
    SendRawData(
        iPadNum,
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

JoypadType SetupHXRealGuitar(int iPadNum, const XINPUT_CAPABILITIES &xcaps) {
    unsigned short us = xcaps.Gamepad.sThumbLY & 0xfffffff0;
    bool u1 = us == 0x1530;
    bool u2 = us == 0x1430;
    if (!u1 && !u2)
        u2 = true;
    if (u1) {
        return kJoypadXboxRealGuitar22Fret;
    } else if (u2) {
        return kJoypadXboxButtonGuitar;
    } else {
        MILO_LOG("sThymbLY = %d does not correspond to subtype x19\n", xcaps.Gamepad.sThumbLY);
        return kJoypadAnalog;
    }
}

JoypadType SetupHXGuitar(int iPadNum, const XINPUT_CAPABILITIES &xcaps) {
    bool u5 = xcaps.Flags & 0x2;
    bool u1 = xcaps.Flags & 1;
    bool u4 = u5 && (u1 || xcaps.Gamepad.sThumbRX >= 0x100);
    JoypadGetPadData(iPadNum)->SetWireless(u5);
    JoypadGetPadData(iPadNum)->SetCanForceFeedback(u1);
    if (xcaps.Gamepad.sThumbLX == 0x1BAD) {
        GetBreedData(iPadNum);
        return kJoypadXboxCoreGuitar;
    } else
        return u4 ? kJoypadXboxHxGuitarRb2 : kJoypadXboxHxGuitar;
}

JoypadType SetupHXDrums(int iPadNum, const XINPUT_CAPABILITIES &xcaps) {
    bool u5 = xcaps.Flags & 0x2;
    bool u1 = xcaps.Flags & 1;
    bool u4 = u5 && (u1 || xcaps.Gamepad.sThumbRX >= 0x100);
    bool u2 = u5 && u1;
    JoypadGetPadData(iPadNum)->SetWireless(u5);
    JoypadGetPadData(iPadNum)->SetCanForceFeedback(u1);
    if (xcaps.Gamepad.sThumbLX == 0x1BAD) {
        GetBreedData(iPadNum);
        return kJoypadXboxMidiBoxDrums;
    } else if (u4) {
        return kJoypadXboxDrumsRb2;
    } else
        return u2 ? kJoypadXboxRoDrums : kJoypadXboxDrums;
}

bool ReceiveUpstreamResponse(int xinput_id, unsigned char *usefulRawStart) {
    switch (usefulRawStart[0]) {
    case 0x80:
        ReceiveUpstreamLowPriorityOutputResponse(xinput_id, usefulRawStart);
        break;
    case 0x82:
        ReceiveUpstreamBreedDataResponse(xinput_id, usefulRawStart);
        break;
    case 0x84:
        ReceiveUpstreamCalbertResponse(xinput_id, usefulRawStart);
        break;
    case 0x86:
        ReceiveUpstreamAccelerometerResponse(xinput_id, usefulRawStart);
        break;
    case 0x8A:
        ReceiveUpstreamOutputModeResponse(xinput_id, usefulRawStart);
        break;
    case 0xC4:
        ReceiveUpstreamDeviceStateResponse(xinput_id, usefulRawStart);
        break;
    case 0xF2:
        ReceiveUpstreamEEPROMReadResponse(xinput_id, usefulRawStart);
        break;
    case 0xF4:
        ReceiveUpstreamEEPROMWriteResponse(xinput_id, usefulRawStart);
        break;
    default:
        return false;
    }
    return true;
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
