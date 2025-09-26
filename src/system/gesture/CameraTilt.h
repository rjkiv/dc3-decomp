#pragma once
#include "obj/Object.h"
#include "os/Timer.h"
#include "ui/UI.h"
#include "xdk/XAPILIB.h"
#include "xdk/NUI.h"

class CameraTilt : public Hmx::Object {
public:
    CameraTilt();
    virtual ~CameraTilt() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void UpdateTiltingToInital();
    void UpdateGetInitialTiltData();
    void StartCameraScan();
    void StartGetInitialTiltData();
    void StartCameraTiltingUp();
    void StartCameraTiltingDown();
    void StartCameraTiltingToInital();
    void Poll();

    static void Init();

protected:
    DataNode OnMsg(const UIChangedMsg &);

    bool unk2c;
    Timer mTimer; // 0x30
    float unk60;
    float mAngle; // 0x64
    int unk68;
    int unk6c;
    int unk70; // 0x70 - some sort of state?
    int unk74;
    int mDelayBetweenStates; // 0x78
    int mDelayBetweenRetry; // 0x7c
    int mUpDownCyclesPerScan; // 0x80
    int mAngleWiggleRoom; // 0x84
    int mErrorRepeatedTimes; // 0x88
    float mCycleSafetyTimeout; // 0x8c
    XOVERLAPPED mOverlapped; // 0x90
    int unkac;
    NUI_TILT_OBJECTS mTiltObjects; // 0xb0
    int unk180;
    int unk184;
    int unk188;
    int unk18c;
};

extern CameraTilt *TheCameraTilt;
