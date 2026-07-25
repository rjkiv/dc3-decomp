#include "gesture/CameraTilt.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "ui/UI.h"
#include "xdk/NUI.h"
#include "xdk/XAPILIB.h"
#include "xdk/nui/nuidetroit.h"
#include "xdk/xapilibi/winerror.h"

CameraTilt *TheCameraTilt;

CameraTilt::CameraTilt()
    : unk2c(0), unk60(0), unk68(0), unk6c(0), unk70(0), unk74(0), mDelayBetweenStates(0),
      mDelayBetweenRetry(0), mUpDownCyclesPerScan(1), mAngleWiggleRoom(3),
      mErrorRepeatedTimes(0), mCycleSafetyTimeout(4), unk180(0) {
    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
    memset(&mTiltObjects, 0, sizeof(NUI_TILT_OBJECTS));
    DataArray *camArr = SystemConfig()->FindArray("camera_tilt", false);
    if (camArr) {
        mDelayBetweenStates =
            camArr->FindInt("delay_between_states", mDelayBetweenStates);
        mDelayBetweenRetry = camArr->FindInt("delay_between_retry", mDelayBetweenRetry);
        mUpDownCyclesPerScan =
            camArr->FindInt("up_down_cycles_per_scan", mUpDownCyclesPerScan);
        mAngleWiggleRoom = camArr->FindInt("angle_wiggle_room", mAngleWiggleRoom);
        mErrorRepeatedTimes =
            camArr->FindInt("error_repeated_times", mErrorRepeatedTimes);
        mCycleSafetyTimeout =
            camArr->FindFloat("cycle_safety_timeout", mCycleSafetyTimeout);
    }
}

BEGIN_HANDLERS(CameraTilt)
    HANDLE_ACTION(camera_scan, StartCameraScan())
    HANDLE_MESSAGE(UIChangedMsg)
END_HANDLERS

BEGIN_PROPSYNCS(CameraTilt)
    SYNC_PROP(angle, mAngle)
END_PROPSYNCS

void CameraTilt::UpdateTiltingToInital() {
    if (mOverlapped.InternalLow != ERROR_IO_PENDING) {
        MILO_LOG("NuiCameraAdjustTilt - completed tilt to inital\n");
        unk70 = 8;
        unk60 = 0;
    }
}

void CameraTilt::UpdateGetInitialTiltData() {
    if (mOverlapped.InternalLow != ERROR_IO_PENDING) {
        MILO_LOG("NuiCameraAdjustTilt - got initial tilt data\n");
        unk70 = 4;
        unk60 = 0;
    }
}

void CameraTilt::Init() {
    MILO_ASSERT(!TheCameraTilt, 0x36);
    TheCameraTilt = new CameraTilt();
    TheCameraTilt->SetName("camera_tilt", ObjectDir::Main());
}

void CameraTilt::StartCameraScan() {
    if (unk70 != 0) {
        MILO_LOG(
            "StartCameraScan: ERROR : Scan is trying to be initiated while in a scan sequence. Ignoring Scan Request!!!"
        );
    } else {
        unk70 = 1;
        unk2c = true;
        unk68 = 0;
        unk60 = 0;
        mTimer.Start();
        ThePlatformMgr.AddSink(TheCameraTilt);
    }
}

void CameraTilt::StartGetInitialTiltData() {
    DWORD ret =
        NuiCameraAdjustTilt(0x20, 0, 2.1336f, 2.1336f, &mTiltObjects, &mOverlapped);
    unk60 = 0;
    if (ret == ERROR_SUCCESS) {
        unk70 = 3;
        MILO_LOG(
            "NuiCameraAdjustTilt completed immediately - camera tilt already optimal?\n"
        );
    } else if (ret == ERROR_IO_PENDING) {
        unk70 = 3;
        if (unk74 == 1) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt - Camera is getting initial camera data\n");
        }
        unk74 = 1;
    } else if (ret == ERROR_RETRY) {
        unk70 = 2;
        if (unk74 == 2) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt called too soon after previous call\n");
        }
        unk74 = 2;
    } else if (ret == ERROR_BUSY) {
        unk70 = 2;
        if (unk74 == 3) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt failed because camera was busy\n");
        }
        unk74 = 3;
    } else if (ret == ERROR_TOO_MANY_CMDS) {
        unk70 = 2;
        if (unk74 == 4) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt failed to find player candidate; waiting\n");
        }
        unk74 = 4;
    } else {
        unk70 = 0;
        if (unk74 == 5) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("Unexpected result from NuiCameraAdjustTilt - %x\n", ret);
        }
        unk74 = 5;
    }
}

void CameraTilt::StartCameraTiltingUp() {
    HRESULT res = NuiCameraElevationSetAngle(27);
    if (res == ERROR_SUCCESS) {
        MILO_LOG("NuiCameraElevationSetAngle - Camera is tilting to Up\n");
        unk70 = 15;
        unk60 = 0;
    } else if (res == E_INVALIDARG) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 6) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the input angle is outside the accepted range\n"
            );
        }
        unk74 = 6;
    } else if (res == E_NUI_DEVICE_NOT_CONNECTED) {
        unk70 = 0;
        unk60 = 0;
        if (unk74 == 7) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Kinect sensor array is not attached\n"
            );
        }
        unk74 = 7;
    } else if (res == E_NUI_SYSTEM_UI_PRESENT) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 8) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Xbox Guide UI is active so elevation will not be changed\n"
            );
        }
        unk74 = 8;
    } else {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 5) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("Unexpected result from NuiCameraElevationSetAngle - %x\n", res);
        }
        unk74 = 5;
    }
}

void CameraTilt::StartCameraTiltingDown() {
    HRESULT res = NuiCameraElevationSetAngle(-27);
    if (res == ERROR_SUCCESS) {
        MILO_LOG("NuiCameraElevationSetAngle - Camera is tilting to Down\n");
        unk70 = 11;
        unk60 = 0;
    } else if (res == E_INVALIDARG) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 6) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the input angle is outside the accepted range\n"
            );
        }
        unk74 = 6;
    } else if (res == E_NUI_DEVICE_NOT_CONNECTED) {
        unk70 = 0;
        unk60 = 0;
        if (unk74 == 7) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Kinect sensor array is not attached\n"
            );
        }
        unk74 = 7;
    } else if (res == E_NUI_SYSTEM_UI_PRESENT) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 8) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Xbox Guide UI is active so elevation will not be changed\n"
            );
        }
        unk74 = 8;
    } else {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 5) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("Unexpected result from NuiCameraElevationSetAngle - %x\n", res);
        }
        unk74 = 5;
    }
}

void CameraTilt::StartCameraTiltingToInital() {
    DWORD res = NuiCameraAdjustTilt(0, 0, 0, 2.1336f, &mTiltObjects, &mOverlapped);
    unk60 = 0;
    if (res == ERROR_SUCCESS) {
        unk70 = 7;
        MILO_LOG(
            "NuiCameraAdjustTilt completed immediately - camera tilt already optimal?\n"
        );
    } else if (res == ERROR_IO_PENDING) {
        unk70 = 7;
        if (unk74 == 1) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt - Camera is getting initial camera data\n");
        }
        unk74 = 1;
    } else if (res == ERROR_RETRY) {
        unk70 = 2;
        if (unk74 == 2) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt called too soon after previous call\n");
        }
        unk74 = 2;
    } else if (res == ERROR_BUSY) {
        unk70 = 2;
        if (unk74 == 3) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt failed because camera was busy\n");
        }
        unk74 = 3;
    } else if (res == ERROR_TOO_MANY_CMDS) {
        unk70 = 2;
        if (unk74 == 4) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("NuiCameraAdjustTilt failed to find player candidate; waiting\n");
        }
        unk74 = 4;
    } else {
        unk70 = 0;
        if (unk74 == 5) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("Unexpected result from NuiCameraAdjustTilt - %x\n", res);
        }
        unk74 = 5;
    }
}

void CameraTilt::UpdateTiltingUp() {
    LONG lAngleDegrees;
    HRESULT res = NuiCameraElevationGetAngle(&lAngleDegrees, &unk180);
    if (res == ERROR_SUCCESS) {
        mAngle = (float)lAngleDegrees * 0.018518519f + 0.5f;
        if (lAngleDegrees > 27 - mAngleWiggleRoom) {
            MILO_LOG("NuiCameraElevationGetAngle : Up : Normal angle end\n");
            unk70 = 16;
            unk60 = 0;
        } else if (unk60 > mCycleSafetyTimeout) {
            MILO_LOG("NuiCameraElevationGetAngle : Up : Safety timer end\n");
            unk70 = 16;
            unk60 = 0;
        }
    } else if (res == E_INVALIDARG) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 6) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the input angle is outside the accepted range\n"
            );
        }
        unk74 = 6;
    } else if (res == E_NUI_DEVICE_NOT_CONNECTED) {
        unk70 = 0;
        unk60 = 0;
        if (unk74 == 7) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Kinect sensor array is not attached\n"
            );
        }
        unk74 = 7;
    } else if (res == E_NUI_SYSTEM_UI_PRESENT) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 8) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Xbox Guide UI is active so elevation will not be changed\n"
            );
        }
        unk74 = 8;
    } else {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 5) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("Unexpected result from NuiCameraElevationSetAngle - %x\n", res);
        }
        unk74 = 5;
    }
}

void CameraTilt::UpdateTiltingDown() {
    LONG lAngleDegrees;
    HRESULT res = NuiCameraElevationGetAngle(&lAngleDegrees, &unk180);
    if (res == ERROR_SUCCESS) {
        mAngle = (float)lAngleDegrees * 0.018518519f + 0.5f;
        if (lAngleDegrees < mAngleWiggleRoom - 27) {
            MILO_LOG("NuiCameraElevationGetAngle : Down : Normal angle end\n");
            unk70 = 12;
            unk60 = 0;
        } else if (unk60 > mCycleSafetyTimeout) {
            MILO_LOG("NuiCameraElevationGetAngle : down : Safety timer end\n");
            unk70 = 12;
            unk60 = 0;
        }
    } else if (res == E_INVALIDARG) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 6) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the input angle is outside the accepted range\n"
            );
        }
        unk74 = 6;
    } else if (res == E_NUI_DEVICE_NOT_CONNECTED) {
        unk70 = 0;
        unk60 = 0;
        if (unk74 == 7) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Kinect sensor array is not attached\n"
            );
        }
        unk74 = 7;
    } else if (res == E_NUI_SYSTEM_UI_PRESENT) {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 8) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG(
                "NuiCameraElevationSetAngle failed because the Xbox Guide UI is active so elevation will not be changed\n"
            );
        }
        unk74 = 8;
    } else {
        unk70 = 5;
        unk60 = 0;
        if (unk74 == 5) {
            unk6c++;
        } else {
            unk6c = 0;
        }
        if (unk6c <= mErrorRepeatedTimes) {
            MILO_LOG("Unexpected result from NuiCameraElevationSetAngle - %x\n", res);
        }
        unk74 = 5;
    }
}

void CameraTilt::Poll() {
    if (!unk2c)
        return;
    switch (unk70) {
    case 0:
        unk2c = false;
        unk68 = 0;
        unk60 = 0;
        mTimer.Stop();
        ThePlatformMgr.RemoveSink(TheCameraTilt);
        break;
    case 1:
        StartGetInitialTiltData();
        break;
    case 2:
        if (unk60 > mDelayBetweenRetry) {
            unk70 = 1;
            unk60 = 0;
        }
        break;
    case 3:
        UpdateGetInitialTiltData();
        break;
    case 4:
        if (unk60 > mDelayBetweenStates) {
            unk70 = 9;
            unk60 = 0;
        }
        break;
    case 5:
        StartCameraTiltingToInital();
        break;
    case 6:
        if (unk60 > mDelayBetweenRetry) {
            unk70 = 5;
            unk60 = 0;
        }
        break;
    case 7:
        UpdateTiltingToInital();
        break;
    case 8:
        if (unk60 > mDelayBetweenStates) {
            unk70 = 0;
            unk60 = 0;
        }
        break;
    case 9:
        StartCameraTiltingDown();
        break;
    case 10:
        if (unk60 > mDelayBetweenRetry) {
            unk70 = 9;
            unk60 = 0;
        }
        break;
    case 11:
        UpdateTiltingDown();
        break;
    case 12:
        if (unk60 > mDelayBetweenStates) {
            unk70 = 13;
            unk60 = 0;
        }
        break;
    case 13:
        StartCameraTiltingUp();
        break;
    case 14:
        if (unk60 > mDelayBetweenRetry) {
            unk70 = 13;
            unk60 = 0;
        }
        break;
    case 15:
        UpdateTiltingUp();
        break;
    case 16:
        if (unk60 > mDelayBetweenStates) {
            unk68++;
            unk60 = 0;
            if (unk68 < mUpDownCyclesPerScan) {
                unk70 = 9;
            } else {
                unk70 = 5;
            }
        }
        break;
    }
    unk60 += Timer::CyclesToMs(mTimer.Stop());
    mTimer.Start();
}

DataNode CameraTilt::OnMsg(const UIChangedMsg &msg) {
    if (unk70 != 0) {
        if (!msg.Showing()) {
            unk2c = true;
            mTimer.Start();
        } else {
            unk2c = false;
            mTimer.Stop();
        }
    }
    return 0;
}
