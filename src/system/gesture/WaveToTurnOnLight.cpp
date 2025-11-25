#include "gesture/WaveToTurnOnLight.h"
#include "hamobj/HamDirector.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "xdk/NUI.h"

WaveToTurnOnLight *TheWaveToTurnOnLight;

WaveToTurnOnLight::WaveToTurnOnLight()
    : mWaveProgress(0), mWaveStateEnabled(0), mPaused(0), mSkeletonID(0) {}

void WaveToTurnOnLight::Init() {
    MILO_ASSERT(!TheWaveToTurnOnLight, 0x1C);
    TheWaveToTurnOnLight = new WaveToTurnOnLight();
    TheWaveToTurnOnLight->SetName("wave_to_turn_on_light", ObjectDir::Main());
}

void WaveToTurnOnLight::PrintError(long err, const char *msg) {
    switch (err) {
    case 0x800704DF:
        MILO_LOG("%s failed with E_NUI_ALREADY_INITIALIZED\n", msg);
        break;
    case 0x8007000E:
        MILO_LOG("%s failed with E_OUTOFMEMORY\n", msg);
        break;
    case 0x80070015:
        MILO_LOG("%s failed with E_NUI_DEVICE_NOT_READY\n", msg);
        break;
    case 0x80070057:
        MILO_LOG("%s failed with E_INVALIDARG\n", msg);
        break;
    case 0x8007048F:
        MILO_LOG("%s failed with E_NUI_DEVICE_NOT_CONNECTED\n", msg);
        break;
    case 0x83010003:
        MILO_LOG("%s failed with E_NUI_IMAGE_STREAM_IN_USE\n", msg);
        break;
    case 0x83010005:
        MILO_LOG("%s failed with E_NUI_FEATURE_NOT_INITIALIZED\n", msg);
        break;
    case 0x8301000D:
        MILO_LOG("%s failed with E_NUI_DATABASE_NOT_FOUND\n", msg);
        break;
    case 0x8301000E:
        MILO_LOG("%s failed with E_NUI_DATABASE_VERSION_MISMATCH\n", msg);
        break;
    default:
        long err_unknown = err;
        MILO_LOG("%s failed with 0x%x\n", msg, err_unknown);
        break;
    }
}

void WaveToTurnOnLight::SetPaused(bool paused) {
    mPaused = paused;
    if (mPaused) {
        mTimer.Pause();
    } else {
        mTimer.Resume();
    }
}

void WaveToTurnOnLight::EnableWaveState() {
    if (!mWaveStateEnabled) {
        HRESULT res = NuiWaveSetEnabled(1);
        if (res == 0) {
            mWaveStateEnabled = true;
            MILO_LOG("Wave detection enabled\n");
            static Message cWaveGestureEnabledMsg("wave_gesture_enabled");
            Export(cWaveGestureEnabledMsg, true);
            mTimer.Restart();
            TheHamDirector->ArmMultiIntroMode();
        } else {
            PrintError(res, "NuiWaveSetEnabled");
        }
        mPaused = false;
    }
}

void WaveToTurnOnLight::DisableWaveState() {
    if (mWaveStateEnabled) {
        HRESULT res = NuiWaveSetEnabled(0);
        if (res == 0) {
            mWaveStateEnabled = false;
            MILO_LOG("Wave detection disabled\n");
            mTimer.Reset();
            static Message cWaveGestureDisabledMsg("wave_gesture_disabled");
            Export(cWaveGestureDisabledMsg, true);
        } else {
            PrintError(res, "NuiWaveSetEnabled");
        }
    }
}

void WaveToTurnOnLight::Poll() {
    bool success = false;
    if (mWaveStateEnabled) {
        HRESULT res = NuiWaveGetGestureOwnerProgress(&mSkeletonID, &mWaveProgress);
        if (res >= 0 && mWaveProgress >= 1) {
            MILO_LOG("You waved your hands!!!\n");
            success = true;
        } else {
            mSkeletonID = 0;
            PrintError(res, "NuiWaveGetGestureOwnerProgress");
        }
    }
    if (mWaveStateEnabled && !success && mTimer.SplitMs() > 40000) {
        MILO_LOG("Wave gesture timed out!!!\n");
        success = true;
    }
    if (success && !mPaused) {
        DisableWaveState();
        static Message waveCompleteMsg("wave_gesture_complete", 0);
    }
}

BEGIN_HANDLERS(WaveToTurnOnLight)
    HANDLE_ACTION(switch_wave_state, SwitchWaveState())
    HANDLE_ACTION(enable_wave_state, EnableWaveState())
    HANDLE_ACTION(disable_wave_state, DisableWaveState())
END_HANDLERS
